#include <iostream>

#include "application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <obj/obj.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <set>

#include <vk_helpers/helpers.hpp>
#include <asset_loader/img/image.h>

#include <Resources/Shaders/PBR_pipeline/pbrpipeline.hpp>

namespace gorilla
{

Application::Application(std::string name, int w, int h, std::vector<uint32_t> vert, std::vector<uint32_t> frag, const engine::scene& scene)
		: _window(std::move(name), w, h), vertCode(std::move(vert)), fragCode(std::move(frag)), scene(scene)
{
	initVulkan();
}

void
Application::initVulkan()
{
	initEnvironment();

	initSwapchain();
	_window.cursorpos_event += [&] (const glfw::window& wnd, double xpos, double ypos)
	{
		if (isMenuShowing)
			return;

		float xshift = xpos;
		xshift /= 10.0f;
		float yshift = -ypos;
		yshift /= 10.0f;
		if (first_mouse)
		{
			lastx = xshift;
			lasty = yshift;

			first_mouse = false;
			return;
		}
		cam.rotate(xshift - lastx, yshift - lasty);
		lastx = xshift;
		lasty = yshift;
	};

	createRenderPass();
	createDescriptorSetLayout();
	// createGraphicsPipeline();
	_pbr_pipeline = std::make_unique<pbr_pipeline>(_device, _render_pass, 0, _scenewise_descriptor_set_layout, _objectwise_descriptor_set_layout);
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	for (int i = 0; i < scene.props.size(); i++)
	{
		const engine::scene::prop_description& desc = scene.props[i];
		props.push_back({});
		engine::texture& albedo = props.back().texture;
		createTextureImage(desc.texture, albedo, vk::Format::eR8G8B8A8Srgb);
		createTextureImageView(albedo, vk::Format::eR8G8B8A8Srgb);
		createTextureSampler(albedo);

		engine::texture& normal = props.back().normal;
		createTextureImage(desc.normal, normal, vk::Format::eR8G8B8A8Unorm);
		createTextureImageView(normal, vk::Format::eR8G8B8A8Unorm);
		createTextureSampler(normal);

		engine::texture& roughness = props.back().roughness;
		createTextureImage(desc.roughness, roughness, vk::Format::eR8G8B8A8Unorm);
		createTextureImageView(roughness, vk::Format::eR8G8B8A8Unorm);
		createTextureSampler(roughness);

		engine::texture& metallic = props.back().metallic;
		createTextureImage(desc.metallic, metallic, vk::Format::eR8G8B8A8Unorm);
		createTextureImageView(metallic, vk::Format::eR8G8B8A8Unorm);
		createTextureSampler(metallic);

		engine::texture& ambient_occlusion = props.back().ambient_occlusion;
		createTextureImage(desc.ambient_occlusion, ambient_occlusion, vk::Format::eR8G8B8A8Unorm);
		createTextureImageView(ambient_occlusion, vk::Format::eR8G8B8A8Unorm);
		createTextureSampler(ambient_occlusion);

		loadModel(scene.props[i].obj_filename, props.back());
		createVertexBuffers(props.back());
		createIndexBuffers(props.back());

		props.back().model = scene.props[i].model;
	}
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();

	createImguiDescriptorPool();
//	ImGui::CreateContext();
//	ImGuiIO& io = ImGui::GetIO();
//	ImGui_ImplGlfw_InitForVulkan(window.handle(), true);
	ImGui_ImplVulkan_InitInfo i{};
	i.Instance = *_instance;
	i.PhysicalDevice = *_phys_device;
	i.Device = *_device;
	i.Queue = *_graphics_queue.vk_queue();
	i.QueueFamily = _graphics_queue.type().index();
	i.DescriptorPool = *ImguiDescriptorPool;
	i.ImageCount = maxFrames;
	i.MinImageCount = maxFrames;
	i.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//	ImGui_ImplVulkan_Init(&i, *renderPass);
	imgui = std::make_unique<engine::misc::imgui_object>(_window.handle(), true, &i,
	                                   *_render_pass);

	auto cmd = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(*cmd);
	endSingleTimeCommands(cmd);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Application::initEnvironment()
{
	_instance = vkh::instance(
		_window.name(),
		VK_MAKE_VERSION(1, 0, 0),
		std::vector<const char*>{
			"VK_LAYER_KHRONOS_validation"
		},
		glfw::window::extensions());

	_phys_device = vkh::most_suitable_device(_instance.enumeratePhysicalDevices());
	std::vector queue_families = vkh::queue_families(_instance, _phys_device);

	std::optional<vk_utils::queue_family> graphics_family = std::nullopt,
		present_family = std::nullopt,
		transfer_family = std::nullopt;

	for (auto qf: queue_families)
	{
		if (!graphics_family.has_value() && qf.has_graphics())
		{
			graphics_family = qf;
			continue;
		}

		if (!present_family.has_value() && qf.has_present())
		{
			present_family = qf;
			continue;
		}

		if (!transfer_family.has_value() && qf.has_transfer())
			transfer_family = qf;
	}

	std::array priorities{ 1.0f };
	std::vector dqcis{
		vk::DeviceQueueCreateInfo({}, graphics_family->index(), priorities),
		vk::DeviceQueueCreateInfo({}, present_family->index(), priorities),
		vk::DeviceQueueCreateInfo({}, transfer_family->index(), priorities),
	};

	_device = vkh::device(
		_phys_device,
		dqcis,
		std::vector{
			"VK_KHR_swapchain"
		},
		_phys_device.getFeatures());
	//sampler anisotropy feature

	_graphics_queue = vkh::get_queue(_device, *graphics_family);
	_present_queue = vkh::get_queue(_device, *present_family);
	_transfer_queue = vkh::get_queue(_device, *transfer_family);

	_surface = _window.surface(_instance);
}

void Application::initSwapchain(bool verbose)
{
	SwapChainSupportDetails details{};

	details.capabilities = _phys_device.getSurfaceCapabilitiesKHR(*_surface);
	details.formats = _phys_device.getSurfaceFormatsKHR(*_surface);
	details.presentModes = _phys_device.getSurfacePresentModesKHR(*_surface);

	std::tie(_sc_format, _sc_cspace) = pickBestFormat(details);
	_sc_present_mode = pickPresentMode(details);

	_sc_extent = details.capabilities.currentExtent;

	uint32_t img_count = std::clamp(2u, details.capabilities.minImageCount, details.capabilities.maxImageCount);

	std::array queue_families{
		_graphics_queue.type().index()
	};
	vk::SwapchainCreateInfoKHR scci(
		{},
		*_surface,
		img_count,
		_sc_format,
		_sc_cspace,
		_sc_extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		queue_families,
		details.capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		_sc_present_mode,
		false);
	_swapchain = _device.createSwapchainKHR(scci);

	std::vector sc_images_raw = _swapchain.getImages();
	std::ranges::transform(sc_images_raw, std::back_inserter(_sc_images), [] (const VkImage& img) -> vk::Image {
		return vk::Image(img);
	});

	createImageViews(verbose);
}

void Application::createImageViews(bool verbose)
{
	_sc_image_views.clear();
	_sc_image_views.reserve(_sc_images.size());
	for (auto& sc_image: _sc_images)
	{
		vk::ImageViewCreateInfo ivci(
				{},
				sc_image,
				vk::ImageViewType::e2D,
				_sc_format,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(
						vk::ImageAspectFlagBits::eColor,
						0,
						1,
						0,
						1));
		_sc_image_views.emplace_back(_device, ivci);
	}
}


void Application::createRenderPass(bool verbose)
{
	vk::SubpassDependency sd(
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	vk::AttachmentDescription colorAttachment(
			{},
			_sc_format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthAttachment(
			{},
			findDepthFormat(),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference ref(
			0,
			vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference dref(
			1,
			vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass(
			{},
			vk::PipelineBindPoint::eGraphics,
			{},
			ref,
			{},
			&dref);

	std::vector attachments = {
			colorAttachment,
			depthAttachment
	};

	vk::RenderPassCreateInfo rpci(
			{},
			attachments,
			subpass,
			sd);

	_render_pass = _device.createRenderPass(rpci);
}

void Application::createDescriptorSetLayout(bool verbose)
{
	std::vector<vk::DescriptorSetLayoutBinding> scenewise_layout = {
			{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
			{ 1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment },
			{ 2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment },
	};

	std::vector<vk::DescriptorSetLayoutBinding> objectwise_layout = {
			{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
			{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
			{ 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
			{ 3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
			{ 4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
			{ 5, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
	};

	vk::DescriptorSetLayoutCreateInfo scenewise_layout_ci({}, scenewise_layout);
	_scenewise_descriptor_set_layout = _device.createDescriptorSetLayout(scenewise_layout_ci);

	vk::DescriptorSetLayoutCreateInfo objectwise_layout_ci({}, objectwise_layout);
	_objectwise_descriptor_set_layout = _device.createDescriptorSetLayout(objectwise_layout_ci);
}

void Application::createGraphicsPipeline(bool verbose)
{
	vk::raii::ShaderModule vert = createShader(vertCode);
	vk::raii::ShaderModule frag = createShader(fragCode);

	vk::PipelineShaderStageCreateInfo pssci_v({}, vk::ShaderStageFlagBits::eVertex, *vert, "main");
	vk::PipelineShaderStageCreateInfo pssci_f({}, vk::ShaderStageFlagBits::eFragment, *frag, "main");

	std::vector<vk::PipelineShaderStageCreateInfo> stages = { pssci_v, pssci_f };

	std::vector<vk::DynamicState> dynStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo pdsci({}, dynStates);

	auto binding = vk_vertex_input_binding_description<struct mesh>(0);
	auto attributes = vk_vertex_input_attribute_description<struct mesh>(0);

	vk::PipelineVertexInputStateCreateInfo pvisci({}, binding, attributes);

	vk::PipelineInputAssemblyStateCreateInfo piaci({}, vk::PrimitiveTopology::eTriangleList, false);

	vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1, {});

	vk::PipelineRasterizationStateCreateInfo resterizer(
			{},
			false,
			false,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise,
			false, 0.0f, 0.0f, 0.0f, 1.0f);

	vk::PipelineDepthStencilStateCreateInfo pdssci(
			{},
			true, true, vk::CompareOp::eLess, false,
			false);

	vk::PipelineMultisampleStateCreateInfo pmci({}, vk::SampleCountFlagBits::e1, false);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment(false);
	colorBlendAttachment.colorWriteMask =
			vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eR;

	vk::PipelineColorBlendStateCreateInfo cbsci(
			{},
			false,
			vk::LogicOp::eCopy,
			colorBlendAttachment,
			{ 0.0f, 0.0f, 0.0f, 0.0f });


	std::vector layouts = {
			*_scenewise_descriptor_set_layout,
			*_objectwise_descriptor_set_layout,
	};
	vk::PipelineLayoutCreateInfo plci({}, layouts);

	pipeline_layout = _device.createPipelineLayout(plci);

	vk::GraphicsPipelineCreateInfo gpci(
			{},
			stages,
			&pvisci,
			&piaci,
			{},
			&viewportState,
			&resterizer,
			&pmci,
			&pdssci,
			&cbsci,
			&pdsci,
			*pipeline_layout,
			*_render_pass,
			0);

	_pipelines = _device.createGraphicsPipelines(nullptr, gpci);
}

void Application::createFramebuffers(bool verbose)
{
	_framebuffer.clear();
	_framebuffer.reserve(_sc_image_views.size());
	for (auto& sc_image_view: _sc_image_views)
	{
		std::vector attachments{
				*sc_image_view,
				*_depth_image_view,
		};

		vk::FramebufferCreateInfo fci(
			{},
			*_render_pass,
			attachments,
			_sc_extent.width,
			_sc_extent.height,
			1);
		_framebuffer.push_back(_device.createFramebuffer(fci));
	}
}

void Application::createCommandPool(bool verbose)
{
	vk::CommandPoolCreateInfo cpci(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		_graphics_queue.type().index());

	_cmd_pool = _device.createCommandPool(cpci);
}

void Application::createDepthResources(bool verbose)
{
	vk::Format format = findDepthFormat();

	std::tie(_depth_image, _depth_image_memory) = createImage(
		_sc_extent.width,
		_sc_extent.height,
		1,
		format,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::ImageViewCreateInfo ivci(
			{},
			*_depth_image,
			vk::ImageViewType::e2D,
			format,
			{},
			{
					vk::ImageAspectFlagBits::eDepth,
					0,
					1,
					0,
					1 });
	_depth_image_view = _device.createImageView(ivci);

	transitionImageLayout(
			*_depth_image,
			format,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			1);
}

void Application::createTextureImage(const asset::image& image, engine::texture& res, vk::Format format)
{
	const vk::DeviceSize image_size = image.width() * image.height() * 4;
	const uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width(), image.height())))) + 1;

	vk::raii::Buffer staging_buffer = nullptr;
	vk::raii::DeviceMemory staging_memory = nullptr;
	std::tie(staging_buffer, staging_memory) = createBuffer(
			image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                                  | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = staging_memory.mapMemory(0, image_size);
	std::memcpy(data, image.data().data(), image_size);
	staging_memory.unmapMemory();


	vk::raii::Image texture_image = nullptr;
	vk::raii::DeviceMemory texture_memory = nullptr;
	std::tie(texture_image, texture_memory) = createImage(
			image.width(),
			image.height(),
			mip_levels,
			format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc
			| vk::ImageUsageFlagBits::eTransferDst
			| vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	transitionImageLayout(
			*texture_image,
			format,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			mip_levels);
	copyBufferToImage(
			staging_buffer,
			texture_image,
			image.width(),
			image.height());
	generateMipmaps(
			*texture_image,
			format,
			image.width(),
			image.height(),
			mip_levels);

	res.image = std::move(texture_image);
	res.mip_levels = mip_levels;
	res.mem = std::move(texture_memory);
}

void Application::createTextureImageView(engine::texture& res, vk::Format format)
{
	vk::ImageViewCreateInfo ivci(
			{},
			*res.image,
			vk::ImageViewType::e2D,
			format,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					res.mip_levels,
					0,
					1));
	res.image_view = _device.createImageView(ivci);
}

void Application::createTextureSampler(engine::texture& res)
{
	vk::PhysicalDeviceProperties properties = _phys_device.getProperties();

	vk::SamplerCreateInfo sci(
			{},
			vk::Filter::eLinear, vk::Filter::eLinear,
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			0.0f,
			true,
			properties.limits.maxSamplerAnisotropy,
			false, vk::CompareOp::eAlways, 0.0f, res.mip_levels,
			vk::BorderColor::eIntOpaqueBlack,
			false);

	res.sampler = _device.createSampler(sci);
}


void Application::loadModel(const std::string& name, engine::scene::prop& res, bool verbose)
{
//	mesh::vertex v;
//	v.position = glm::vec3(-1.0f, -1.0f, 0.0f);
//	v.texcoord = glm::vec3(0.0f, 0.0f, 0.0f);
//	v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
//	res.mesh.vertices.push_back(v);
//	v.position = glm::vec3(1.0f, -1.0f, 0.0f);
//	v.texcoord = glm::vec3(2.0f, 0.0f, 0.0f);
//	res.mesh.vertices.push_back(v);
//	v.position = glm::vec3(1.0f, 1.0f, 0.0f);
//	v.texcoord = glm::vec3(2.0f, 2.0f, 0.0f);
//	res.mesh.vertices.push_back(v);
//	v.position = glm::vec3(-1.0f, 1.0f, 0.0f);
//	v.texcoord = glm::vec3(0.0f, 2.0f, 0.0f);
//	res.mesh.vertices.push_back(v);
//	res.mesh.indices = {
//			0, 1, 2,
//			0, 2, 3,
//	};
//	calculate_tangent_space(res.mesh);
//	return;

	asset::obj obj = asset::obj::load_obj(name);
	res.mesh = mesh::from_obj(obj);
//	obj = asset::triangulate_obj(obj);
//
//	std::map<std::pair<int32_t, int32_t>, int32_t> vertices;
//
//	std::vector<std::set<int>> vertex_to_triangle(obj.n_vertices());
//
//	for (const asset::obj::index_triplet& t : obj.face_indices())
//	{
//		std::pair<int32_t, int32_t> key = { t.vi, t.vti };
//
//		const auto& thing = vertices.find(key);
//
//		if (thing == vertices.end())
//		{
//			mesh::vertex v{};
//			v.position = obj.vertices()[t.vi];
//			v.texcoord = obj.texcoords()[t.vti];
//			v.normal = obj.normals()[t.vni];
////			v.texcoord.y = 1.0f - v.texcoord.y;
//
//			vertices.insert_or_assign(key, res.mesh.vertices.size());
//			res.mesh.vertices.push_back(v);
//		}
//		res.mesh.indices.push_back(vertices.at(key));
//		vertex_to_triangle[t.vi].insert(vertices.at(key));
//	}
//
//	std::vector<std::vector<int>> triangle_to_vertex(obj.n_face_indices());
//	for (int i = 0; i < vertex_to_triangle.size(); i++)
//	{
//		for (auto iter = vertex_to_triangle[i].begin(); iter != vertex_to_triangle[i].end(); iter++)
//		{
//			for (auto inter = vertex_to_triangle[i].begin(); inter != vertex_to_triangle[i].end(); inter++)
//			{
//				triangle_to_vertex[*iter].push_back(*inter);
//			}
//		}
//	}
//	calculate_tangent_space(res.mesh);
}

void Application::createVertexBuffers(engine::scene::prop& res, bool verbose)
{
	const vk::DeviceSize size = sizeof(res.mesh.vertices[0]) * res.mesh.vertices.size();

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.vertices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.geom_buffer.vertex_buffer = std::move(buffers.first);
	res.geom_buffer.vertex_buffer_memory = std::move(buffers.second);

	copyBuffer(
			stagingBuffer,
			res.geom_buffer.vertex_buffer,
			size);
}

void Application::createIndexBuffers(engine::scene::prop& res, bool verbose)
{
	const vk::DeviceSize size = sizeof(res.mesh.indices[0]) * res.mesh.indices.size();

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.indices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	res.geom_buffer.index_buffer = std::move(buffers.first);
	res.geom_buffer.index_buffer_memory = std::move(buffers.second);

	copyBuffer(
			stagingBuffer,
			res.geom_buffer.index_buffer,
			size);
}

void Application::createSkybox()
{
	vk::raii::Image img = nullptr;
	vk::raii::DeviceMemory mem = nullptr;

	vk::ImageCreateInfo ici(
			{},
			vk::ImageType::e2D,
			vk::Format::eR8G8B8A8Srgb,
			{ 900, 900, 1 },
			1,
			6,
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined);
	img = _device.createImage(ici);

	vk::ImageMemoryRequirementsInfo2 imri(*img);
	vk::MemoryRequirements2 memRequirements = _device.getImageMemoryRequirements2(imri);

	vk::MemoryAllocateInfo allocInfo(
			memRequirements.memoryRequirements.size,
			findMemoryType(
					memRequirements.memoryRequirements.memoryTypeBits,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	mem = _device.allocateMemory(allocInfo);

	vk::BindImageMemoryInfo bimi(*img, *mem, 0);
	_device.bindImageMemory2(bimi);

	vk::ImageViewCreateInfo ivci(
		{},
		*img,
		vk::ImageViewType::eCubeArray,
		vk::Format::eR8G8B8A8Srgb,
		vk::ComponentMapping{},
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			6));
	vk::raii::ImageView view = _device.createImageView(ivci);

	vk::SamplerCreateInfo sci(
		{},
		vk::Filter::eNearest,
		vk::Filter::eNearest,
		vk::SamplerMipmapMode::eNearest,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat);
	vk::raii::Sampler sampler = _device.createSampler(sci);
}

void Application::createUniformBuffers(bool verbose)
{
	// view_projection
	{
		const vk::DeviceSize size = sizeof(view_projection);

		auto&& buffers = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		view_projection_buffer = std::move(buffers.first);
		view_projection_buffer_memory = std::move(buffers.second);
	}

	// light_sources
	{
		const vk::DeviceSize alignment = _phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
		const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
		const vk::DeviceSize size = padded_size + sizeof(engine::lights::spot_lights_t);

		auto&& buffers = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		light_sources_buffer = std::move(buffers.first);
		light_sources_buffer_memory = std::move(buffers.second);
	}

	// models
	const vk::DeviceSize size = props.size() * sizeof(glm::mat4);

	auto&& buffers = createBuffer(
			size,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	models_buffer = std::move(buffers.first);
	models_buffer_memory = std::move(buffers.second);
}

void Application::createDescriptorPool(bool verbose)
{
	std::vector<vk::DescriptorPoolSize> poolSizes = {
			{ vk::DescriptorType::eUniformBuffer,        (uint32_t)(3 * maxFrames + maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
	};

	vk::DescriptorPoolCreateInfo dpci(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxFrames * (1 + props.size()), poolSizes);

	descriptorPool = _device.createDescriptorPool(dpci);
}

void Application::createImguiDescriptorPool(bool verbose)
{
	std::vector<vk::DescriptorPoolSize> poolSizes = {
			{ vk::DescriptorType::eUniformBuffer,        1000},
			{ vk::DescriptorType::eCombinedImageSampler, 1000},
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 },
	};

	vk::DescriptorPoolCreateInfo dpci(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,  1000, poolSizes);

	ImguiDescriptorPool = _device.createDescriptorPool(dpci);
}

void Application::createDescriptorSets(bool verbose)
{
	std::vector<vk::DescriptorSetLayout> counts(1, *_scenewise_descriptor_set_layout);
	vk::DescriptorSetAllocateInfo dsai(*descriptorPool, counts);

	_scenewise_descriptor_set = std::move(_device.allocateDescriptorSets(dsai).front());

	counts = std::vector<vk::DescriptorSetLayout>(props.size(), *_objectwise_descriptor_set_layout);
	dsai = vk::DescriptorSetAllocateInfo(*descriptorPool, counts);

	objectwise_descriptor_sets = _device.allocateDescriptorSets(dsai);

	vk::DescriptorBufferInfo view_projection_info(*view_projection_buffer, 0, sizeof(view_projection));
	vk::DescriptorBufferInfo point_lights_info(*light_sources_buffer, 0, sizeof(engine::lights::point_lights_t));

	const vk::DeviceSize alignment = _phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
	const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
	vk::DescriptorBufferInfo spot_lights_info(*light_sources_buffer, padded_size, sizeof(engine::lights::spot_lights_t));

	std::vector<vk::WriteDescriptorSet> writes = {
			{*_scenewise_descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, view_projection_info},
			{*_scenewise_descriptor_set, 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, point_lights_info},
			{*_scenewise_descriptor_set, 2, 0, vk::DescriptorType::eUniformBuffer, nullptr, spot_lights_info},
	};

	_device.updateDescriptorSets(writes, {});

	for (int i = 0; i < props.size(); i++)
	{
		vk::DescriptorBufferInfo model_info(*models_buffer, i * sizeof(glm::mat4), sizeof(glm::mat4));

		vk::DescriptorImageInfo texture_info(*props[i].texture.sampler, *props[i].texture.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo normal_info(*props[i].normal.sampler, *props[i].normal.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo roughness_info(*props[i].roughness.sampler, *props[i].roughness.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo metallic_info(*props[i].metallic.sampler, *props[i].metallic.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo ao_info(*props[i].ambient_occlusion.sampler, *props[i].ambient_occlusion.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);

		std::vector<vk::WriteDescriptorSet> descriptorWrites = {
				{*objectwise_descriptor_sets[i], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, model_info},
				{*objectwise_descriptor_sets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, texture_info},
				{*objectwise_descriptor_sets[i], 2, 0, vk::DescriptorType::eCombinedImageSampler, normal_info},
				{*objectwise_descriptor_sets[i], 3, 0, vk::DescriptorType::eCombinedImageSampler, roughness_info},
				{*objectwise_descriptor_sets[i], 4, 0, vk::DescriptorType::eCombinedImageSampler, metallic_info},
				{*objectwise_descriptor_sets[i], 5, 0, vk::DescriptorType::eCombinedImageSampler, ao_info},
		};

		_device.updateDescriptorSets(descriptorWrites, {});
	}
}

void Application::createCommandBuffers(bool verbose)
{
	vk::CommandBufferAllocateInfo cbai(*_cmd_pool, vk::CommandBufferLevel::ePrimary, maxFrames);
	commandBuffers = _device.allocateCommandBuffers(cbai);
}

void Application::createSyncObjects(bool verbose)
{
	vk::SemaphoreCreateInfo sci;

	vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);

	for (int i = 0; i < maxFrames; i++)
	{
		imageAvailableSemaphores.push_back(_device.createSemaphore(sci));
		renderFinishedSemaphores.push_back(_device.createSemaphore(sci));
		isFlightFences.push_back(_device.createFence(fci));
	}
}

void Application::recreateSwapChain(bool verbose)
{
	vk::Extent2D ext = _window.framebuffer_size();
	while (ext.height == 0 || ext.width == 0)
	{
		ext = _window.framebuffer_size();
		glfw::window::wait_events();
	}

	_device.waitIdle();

	_framebuffer.clear();
	_sc_image_views.clear();
	_swapchain = nullptr;

	initSwapchain();
	createDepthResources();
	createFramebuffers();
}


void Application::show_meshes()
{

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	bool node_open = ImGui::TreeNode("Meshes");

	if (!node_open)
		return;

	for (int i = 0; i < scene.props.size(); i++)
	{
		ImGui::PushID(i);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		bool leaf_open = ImGui::TreeNode("Meshes", "%s %d", "Mesh", i + 1);

		if (!leaf_open)
		{
			ImGui::NextColumn();
			ImGui::PopID();
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGuiTreeNodeFlags property_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

		ImGui::TreeNodeEx("Translation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputFloat3("##trans", (float*)&props[i].trans);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Angles", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputFloat3("##angles", (float*)&props[i].angles);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Scale", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputFloat("##scale", &props[i].scale);
		ImGui::NextColumn();

		ImGui::TreePop();
		ImGui::PopID();
	}

	ImGui::TreePop();
}

void Application::show_point_lights()
{

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	bool node_open = ImGui::TreeNode("Point Lights");

	if (!node_open)
		return;

	for (int i = 0; i < scene.lights.point_lights.n_lights; i++)
	{
		ImGui::PushID(i);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		bool leaf_open = ImGui::TreeNode("Point Light", "%s %d", "Point Light", i + 1);

		if (!leaf_open)
		{
			ImGui::NextColumn();
			ImGui::PopID();
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGuiTreeNodeFlags property_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

		ImGui::TreeNodeEx("Position", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat3("##position", (float*)&scene.lights.point_lights.lights[i].position, 0.1f, -5.0f, 5.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Color", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGuiColorEditFlags flags1 = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB;
		ImGui::ColorEdit3("##color", (float*)&scene.lights.point_lights.lights[i].color, flags1);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Intensity", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##intensity", (float*)&scene.lights.point_lights.lights[i].intensity, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Constant Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##constant", &scene.lights.point_lights.lights[i].constant, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Linear Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##linear", &scene.lights.point_lights.lights[i].linear, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Quadratic Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##quadratic", &scene.lights.point_lights.lights[i].quadratic, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TreePop();
		ImGui::PopID();
	}

	ImGui::TreePop();
}

void Application::show_spot_lights()
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	bool node_open = ImGui::TreeNode("Spot Lights");

	if (!node_open)
		return;

	for (int i = 0; i < scene.lights.spot_lights.n_lights; i++)
	{
		ImGui::PushID(i);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		bool leaf_open = ImGui::TreeNode("Spot Light", "%s %d", "Spot Light", i + 1);

		if (!leaf_open)
		{
			ImGui::NextColumn();
			ImGui::PopID();
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGuiTreeNodeFlags property_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

		ImGui::TreeNodeEx("Position", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat3("##position", (float*)&scene.lights.spot_lights.lights[i].position, 0.1f, -5.0f, 5.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Direction", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat3("##direction", (float*)&scene.lights.spot_lights.lights[i].direction, 0.1f, -5.0f, 5.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Width", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat("##width", &scene.lights.spot_lights.lights[i].cone_size, 0.01f, 0.3f, 0.99f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Color", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGuiColorEditFlags flags1 = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB;
		ImGui::ColorEdit3("##color", (float*)&scene.lights.spot_lights.lights[i].color, flags1);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Intensity", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat("##intensity", &scene.lights.spot_lights.lights[i].intensity, 0.01f, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Constant Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##constant", &scene.lights.spot_lights.lights[i].constant, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Linear Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##linear", &scene.lights.spot_lights.lights[i].linear, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TreeNodeEx("Quadratic Attenuation", property_flags);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SliderFloat("##quadratic", &scene.lights.spot_lights.lights[i].quadratic, 0.0f, 10.0f);
		ImGui::NextColumn();

		ImGui::TreePop();
		ImGui::PopID();
	}

	ImGui::TreePop();
}

void Application::show_control_window()
{
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 3.5f, main_viewport->Size.y), ImGuiCond_Always);

	if (!ImGui::Begin("Control", NULL, flags))
	{
		ImGui::End();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		show_meshes();
		show_point_lights();
		show_spot_lights();
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();
	ImGui::End();
}


void Application::drawFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (isMenuShowing)
	{
		show_control_window();
//		ImGui::ShowDemoWindow();
	}


	ImGui::Render();

	glfw::key_action action = _window.get_key(glfw::key_id::A);
	if (action == glfw::key_action::press)
	{
		cam.position -= cam.right / 30.0f;
	}
	action = _window.get_key(glfw::key_id::D);
	if (action == glfw::key_action::press)
	{
		cam.position += cam.right / 30.0f;
	}
	action = _window.get_key(glfw::key_id::W);
	if (action == glfw::key_action::press)
	{
		cam.position += cam.fwd / 30.0f;
	}
	action = _window.get_key(glfw::key_id::S);
	if (action == glfw::key_action::press)
	{
		cam.position -= cam.fwd / 30.0f;
	}
	action = _window.get_key(glfw::key_id::escape);
	if (action == glfw::key_action::press)
	{
		if (!lockChange && !isMenuShowing)
		{
			_window.set_input_mode(GLFW_CURSOR_NORMAL);
			isMenuShowing = true;
		}
		else if (!lockChange)
		{
			_window.set_input_mode(GLFW_CURSOR_DISABLED);
			isMenuShowing = false;
		}
		lockChange = true;
	}
	if (action == glfw::key_action::release)
	{
		lockChange = false;
	}

	vk::Result result = _device.waitForFences(*isFlightFences[currentFrame], true, UINT64_MAX);
	resultCheck(result, "Fence checker");

	uint32_t imgIndex;
	vk::AcquireNextImageInfoKHR anii(*_swapchain, UINT64_MAX, *imageAvailableSemaphores[currentFrame], {}, 1);
	auto tmp = _device.acquireNextImage2KHR(anii);
	result = tmp.first;
	if (result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		frameBufferResized = false;
		recreateSwapChain();
		return;
	}
	_device.resetFences(*isFlightFences[currentFrame]);
	imgIndex = tmp.second;

	commandBuffers[currentFrame].reset();
	recordCommandBuffer(commandBuffers[currentFrame], imgIndex);

	std::array<vk::PipelineStageFlags, 1> waitStages{
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
	};
	vk::SubmitInfo si(
			*imageAvailableSemaphores[currentFrame], waitStages, *commandBuffers[currentFrame],
			*renderFinishedSemaphores[currentFrame]);

	updateUniformBuffer(currentFrame);

	_graphics_queue.vk_queue().submit(si, *isFlightFences[currentFrame]);


	vk::PresentInfoKHR pi(*renderFinishedSemaphores[currentFrame], *_swapchain, imgIndex);

	try
	{
		_present_queue.vk_queue().presentKHR(pi);
		currentFrame = (currentFrame + 1) % maxFrames;
	}
	catch (...)
	{
		frameBufferResized = false;
		recreateSwapChain();
		currentFrame = (currentFrame + 1) % maxFrames;
	}
}

void Application::recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const
{
	vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	buffer.begin(cbbi);

	std::array<float, 4> clr = { 0.1f, 0.1f, 0.1f, 1.0f };
	std::vector<vk::ClearValue> clearColors(2);
	clearColors[0].color = clr;
	clearColors[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };
	vk::RenderPassBeginInfo rpbi(
			*_render_pass,
			*_framebuffer[imgIndex],
			{{ 0, 0 }, _sc_extent },
			clearColors);
	buffer.beginRenderPass(rpbi, vk::SubpassContents::eInline);
	{
		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pbr_pipeline->pipeline());

		vk::Viewport vp(
				0.0f, 0.0f,
				static_cast<float>(_sc_extent.width), static_cast<float>(_sc_extent.height), 0.0f,
				1.0f);
		buffer.setViewport(0, vp);

		vk::Rect2D sc({ 0, 0 }, _sc_extent);
		buffer.setScissor(0, sc);

		buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pbr_pipeline->layout(), 0, *_scenewise_descriptor_set, {});

		for (int i = 0; i < props.size(); i++)
		{
			std::vector<vk::DeviceSize> ds = { 0 };
			buffer.bindVertexBuffers(0, *props[i].geom_buffer.vertex_buffer, ds);
			buffer.bindIndexBuffer(*props[i].geom_buffer.index_buffer, 0, vk::IndexType::eUint32);

			buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pbr_pipeline->layout(), 1, *objectwise_descriptor_sets[i], {});

			buffer.drawIndexed(props[i].mesh.indices.size(), 1, 0, 0, 0);
		}

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *buffer);
	}
	buffer.endRenderPass();
	buffer.end();
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


	view_proj.view = cam.view();
	view_proj.projection = glm::perspective(
			glm::radians(60.0f), _sc_extent.width / float(_sc_extent.height), 0.1f, 400.0f);
	view_proj.projection[1][1] *= -1;
	view_proj.camPos = cam.position;

	glm::mat4 r = glm::mat4(1.0f);
//	r = glm::rotate(r, glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	scene.lights.point_lights.lights[0].position = r * glm::vec4(scene.lights.point_lights.lights[0].position, 1.0f);

	scene.lights.spot_lights.lights[0].position = glm::vec4(cam.position, 0.0f);
	scene.lights.spot_lights.lights[0].direction = glm::normalize(glm::vec4(cam.fwd, 0.0f));

	for (engine::scene::prop& p : props)
	{
		p.model = glm::mat4(1.0f);
		p.model = glm::translate(p.model, p.trans);
		p.model = glm::scale(p.model, glm::vec3(p.scale));
		p.model = glm::rotate(p.model, glm::radians(p.angles.x), glm::vec3(1.0f, 0.0f, 0.0f));
		p.model = glm::rotate(p.model, glm::radians(p.angles.y), glm::vec3(0.0f, 1.0f, 0.0f));
		p.model = glm::rotate(p.model, glm::radians(p.angles.z), glm::vec3(0.0f, 0.0f, 1.0f));

	}

	// view_projeciton
	{
		void* data = view_projection_buffer_memory.mapMemory(0, sizeof(view_projection));
		std::memcpy(data, &view_proj, sizeof(view_projection));
		view_projection_buffer_memory.unmapMemory();
	}

	// light_sources
	{
		void* data = light_sources_buffer_memory.mapMemory(0, sizeof(engine::lights::point_lights_t));
		std::memcpy(data, &scene.lights.point_lights, sizeof(engine::lights::point_lights_t));
		light_sources_buffer_memory.unmapMemory();

		const vk::DeviceSize alignment = _phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
		const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
		data = light_sources_buffer_memory.mapMemory(padded_size, sizeof(engine::lights::spot_lights_t));
		std::memcpy(data, &scene.lights.spot_lights, sizeof(engine::lights::spot_lights_t));
		light_sources_buffer_memory.unmapMemory();
	}

	// models
	{
		void* data = nullptr;

		for (int i = 0; i < props.size(); i++)
		{
			data = models_buffer_memory.mapMemory(i * sizeof(glm::mat4), sizeof(glm::mat4));
			std::memcpy(data, &props[i].model, sizeof(glm::mat4));
			models_buffer_memory.unmapMemory();
		}
	}
}


void Application::run()
{
	while (!_window.should_close())
	{
		glfw::window::poll_events();
		drawFrame();
	}

	_device.waitIdle();
}

std::pair<vk::Format, vk::ColorSpaceKHR>
Application::pickBestFormat(const Application::SwapChainSupportDetails& details)
{
	vk::Format format = details.formats[0].format;
	vk::ColorSpaceKHR color_space = details.formats[0].colorSpace;

	return { format, color_space };
}

vk::PresentModeKHR
Application::pickPresentMode(const Application::SwapChainSupportDetails& details)
{
	return vk::PresentModeKHR::eFifo;
}


uint32_t Application::findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties mp = _phys_device.getMemoryProperties();
	for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
	{
		if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Nu suitable memory type found!");
}

vk::Format Application::findSupportedFormat(
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features)
{
	for (vk::Format format: candidates)
	{
		vk::FormatProperties props = _phys_device.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features
		    || tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

vk::Format Application::findDepthFormat()
{
	return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}


vk::raii::ShaderModule Application::createShader(const std::vector<uint32_t>& code)
{
	vk::ShaderModuleCreateInfo smci(
			{},
			4 * code.size(),
			code.data());

	return _device.createShaderModule(smci);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Application::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	vk::raii::Buffer resBuf = nullptr;
	vk::raii::DeviceMemory resBufMem = nullptr;
	vk::BufferCreateInfo bci({}, size, usage, vk::SharingMode::eExclusive);
	resBuf = _device.createBuffer(bci);

	vk::MemoryRequirements mr = resBuf.getMemoryRequirements();

	vk::MemoryAllocateInfo mai(mr.size, findMemoryType(mr.memoryTypeBits, properties));
	resBufMem = _device.allocateMemory(mai);

	resBuf.bindMemory(*resBufMem, 0);

	return { std::move(resBuf), std::move(resBufMem) };
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
Application::createImage(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties)
{
	vk::raii::Image img = nullptr;
	vk::raii::DeviceMemory mem = nullptr;

	vk::ImageCreateInfo ici(
			{},
			vk::ImageType::e2D,
			format,
			{ width, height, 1 },
			mipLevels,
			1,
			vk::SampleCountFlagBits::e1,
			tiling,
			usage,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined);
	img = _device.createImage(ici);

	vk::ImageMemoryRequirementsInfo2 imri(*img);
	vk::MemoryRequirements2 memRequirements = _device.getImageMemoryRequirements2(imri);

	vk::MemoryAllocateInfo allocInfo(
			memRequirements.memoryRequirements.size,
			findMemoryType(
					memRequirements.memoryRequirements.memoryTypeBits,
					properties));
	mem = _device.allocateMemory(allocInfo);

	vk::BindImageMemoryInfo bimi(*img, *mem, 0);
	_device.bindImageMemory2(bimi);

	return { std::move(img), std::move(mem) };
}


std::vector<char>
Application::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::exception("Cannot read file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}


vk::raii::CommandBuffer Application::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo cbai(*_cmd_pool, vk::CommandBufferLevel::ePrimary, 1);

	vk::raii::CommandBuffer cb = std::move(_device.allocateCommandBuffers(cbai)[0]);

	vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cb.begin(cbbi);

	return cb;
}

void Application::endSingleTimeCommands(const vk::raii::CommandBuffer& cmd)
{
	cmd.end();

	vk::SubmitInfo si({}, {}, *cmd);
	_graphics_queue.vk_queue().submit(si);
	_graphics_queue.vk_queue().waitIdle();
}


void Application::copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands();

	vk::BufferCopy bc(0, 0, size);
	cb.copyBuffer(*srcBuffer, *dstBuffer, bc);

	endSingleTimeCommands(cb);
}

void
Application::copyBufferToImage(
		const vk::raii::Buffer& buffer,
		const vk::raii::Image& image,
		uint32_t width,
		uint32_t height)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands();

	vk::BufferImageCopy bic(
			0, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, { width, height, 1 });

	cb.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, bic);

	endSingleTimeCommands(cb);
}


void Application::transitionImageLayout(
		vk::Image image,
		vk::Format format,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t mipLevels)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands();

	vk::ImageMemoryBarrier barrier(
			{}, {}, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED, image,
			{ vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1 });

	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	vk::PipelineStageFlags srcStage;
	vk::PipelineStageFlags dstStage;
	if (oldLayout == vk::ImageLayout::eUndefined
	    && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
	         && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined
	         && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		throw std::invalid_argument("unsupported pipeline_layout transition!");
	}

	cb.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

	endSingleTimeCommands(cb);
}

void
Application::generateMipmaps(
		vk::Image img,
		vk::Format imageFormat,
		int32_t texW,
		int32_t texH,
		uint32_t mipLevels)
{
	vk::FormatProperties propertiess = _phys_device.getFormatProperties(imageFormat);
	if (!(propertiess.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	vk::raii::CommandBuffer buffer = beginSingleTimeCommands();

	vk::ImageMemoryBarrier barrier(
			{}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, img,
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	int32_t mipW = texW;
	int32_t mipH = texH;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		buffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

		std::array<vk::Offset3D, 2> srcOffsets = {
				vk::Offset3D(0, 0, 0),
				vk::Offset3D(mipW, mipH, 1),
		};
		std::array<vk::Offset3D, 2> dstOffsets = {
				vk::Offset3D(0, 0, 0),
				vk::Offset3D(mipW > 1 ? mipW / 2 : 1, mipH > 1 ? mipH / 2 : 1, 1),
		};
		vk::ImageBlit blit(
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1), srcOffsets,
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1), dstOffsets);
		buffer.blitImage(
				img, vk::ImageLayout::eTransferSrcOptimal, img, vk::ImageLayout::eTransferDstOptimal, blit,
				vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		buffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
				barrier);

		if (mipW > 1) mipW /= 2;
		if (mipH > 1) mipH /= 2;

	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

	endSingleTimeCommands(buffer);
}


bool Application::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}


} // gorilla