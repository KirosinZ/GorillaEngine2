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

namespace gorilla
{

Application::Application(std::string name, int w, int h, std::vector<uint32_t> vert, std::vector<uint32_t> frag, const engine::scene& scene)
		: window(std::move(name), w, h), vertCode(std::move(vert)), fragCode(std::move(frag)), scene(scene)
{
	initVulkan();
}

void
Application::initVulkan()
{
	environment = initEnvironment(window);

	initSwapchain();
	window.cursorpos_event += [&] (const glfw::window& wnd, double xpos, double ypos)
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
	createGraphicsPipeline();
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
	i.Instance = *environment.instance();
	i.PhysicalDevice = *environment.phys_device();
	i.Device = *environment.device();
	i.Queue = *environment.graphics_queue();
	i.QueueFamily = environment.graphics_queue_index();
	i.DescriptorPool = *ImguiDescriptorPool;
	i.ImageCount = maxFrames;
	i.MinImageCount = maxFrames;
	i.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//	ImGui_ImplVulkan_Init(&i, *renderPass);
	imgui = std::make_unique<engine::misc::imgui_object>(window.handle(), true, &i,
	                                   *renderPass);

	auto cmd = beginSingleTimeCommands(environment, commandPool);
	ImGui_ImplVulkan_CreateFontsTexture(*cmd);
	endSingleTimeCommands(environment, cmd);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

vk_utils::environment
Application::initEnvironment(const glfw::window& window)
{
	vk::raii::Instance instance = createInstance(vk_utils::environment::context(), window);
	std::vector<vk::raii::SurfaceKHR> surfaces;
	surfaces.push_back(createSurface(window, instance));
	int graphics_family = -1;
	int present_family = -1;
	vk::raii::PhysicalDevice phys_device    = createPhysDevice(instance, surfaces[0], graphics_family, present_family);
	vk::raii::Device device = createLogicalDevice(phys_device, graphics_family, present_family);
	vk::raii::Queue graphics_queue = createQueue(device, graphics_family);
	vk::raii::Queue present_queue  = createQueue(device, present_family);

	return vk_utils::environment(
			std::move(instance),
			std::move(phys_device),
			std::move(device),
			graphics_family,
			std::move(graphics_queue),
			std::move(surfaces),
			present_family,
			std::move(present_queue));
}

vk::raii::Instance
Application::createInstance(const vk::raii::Context& context, const glfw::window& window)
{
	vk::ApplicationInfo ai(
			window.name().c_str(),
			VK_MAKE_API_VERSION(1, 0, 1, 0),
			Engine::Name.c_str(),
			Engine::Version,
			Engine::VulkanVersion
	);

	const std::vector<const char*> layers{

	};

	std::vector<const char*> extensions = glfw::window::extensions();

	vk::InstanceCreateInfo ici({}, &ai, layers, extensions);

	return context.createInstance(ici);
}

vk::raii::SurfaceKHR
Application::createSurface(const glfw::window& window, const vk::raii::Instance& instance)
{
	return window.surface(instance);
}

vk::raii::PhysicalDevice
Application::createPhysDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, int& graphics_family, int& present_family)
{
	vk::raii::PhysicalDevice               phys_device    = pickDevice(instance.enumeratePhysicalDevices());
	std::vector<vk::QueueFamilyProperties> queue_families = phys_device.getQueueFamilyProperties();

	for (int i = 0; i < queue_families.size(); ++i)
	{
		if (queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphics_family = i;
		}

		if (phys_device.getSurfaceSupportKHR(i, *surface))
		{
			present_family = i;
		}
	}

	return std::move(phys_device);
}

vk::raii::Device
Application::createLogicalDevice(const vk::raii::PhysicalDevice& phys_device, int graphics_family, int present_family)
{
	const std::array<float, 1> priorities = { 1.0f };

	vk::DeviceQueueCreateInfo dqci({}, graphics_family, priorities);
	vk::DeviceQueueCreateInfo dqci2({}, present_family, priorities);

	std::vector<vk::DeviceQueueCreateInfo> dqcis = {
			dqci,
			dqci2
	};

	std::vector<const char*> deviceLayers{

	};

	std::vector<const char*> deviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;

	vk::DeviceCreateInfo dci({}, dqcis, deviceLayers, deviceExtensions, &features);

	return phys_device.createDevice(dci);
}

vk::raii::Queue
Application::createQueue(const vk::raii::Device& device, int index)
{
	return device.getQueue(index, 0);
}


void
Application::initSwapchain(bool verbose)
{
	createSwapChain(verbose);
	createImageViews(verbose);
}

void
Application::createSwapChain(bool verbose)
{
	Application::SwapChainSupportDetails details = querySupport(environment);
	vk::Format format;
	vk::ColorSpaceKHR colorSpace;
	std::tie(format, colorSpace) = pickBestFormat(details);
	vk::PresentModeKHR presentMode = pickPresentMode(details);


	vk::Extent2D extent = window.framebuffer_size();
	vk::Extent2D actualExtent = extent;
	actualExtent.width = std::clamp(
			actualExtent.width, details.capabilities.minImageExtent.width,
			details.capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(
			actualExtent.height, details.capabilities.minImageExtent.height,
			details.capabilities.maxImageExtent.height);

	std::array<uint32_t, 2> bl = { uint32_t(environment.graphics_queue_index()), uint32_t (environment.present_queue_index()) };
	vk::SwapchainCreateInfoKHR scci(
			{},
			*environment.surface(0),
			3,
			format,
			colorSpace,
			actualExtent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eConcurrent,
			bl,
			details.capabilities.currentTransform,
			//vk::SurfaceTransformFlagBitsKHR::eIdentity,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode,
			true);
	swapchain = vk::raii::SwapchainKHR(environment.device(), scci);
	auto imgs = swapchain.getImages();
	scImages.clear();
	for (auto img: imgs)
	{
		scImages.emplace_back(img);
	}
	scImageExtent = actualExtent;
	scImageFormat = format;
}

void
Application::createImageViews(bool verbose)
{
	imageViews.clear();
	imageViews.reserve(scImages.size());
	for (auto& scImage: scImages)
	{
		vk::ImageViewCreateInfo ivci(
				{},
				scImage,
				vk::ImageViewType::e2D,
				scImageFormat,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(
						vk::ImageAspectFlagBits::eColor,
						0,
						1,
						0,
						1));
		imageViews.emplace_back(environment.device(), ivci);
	}
}


void Application::createRenderPass(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::SubpassDependency sd(
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	vk::AttachmentDescription colorAttachment(
			{},
			scImageFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthAttachment(
			{},
			findDepthFormat(environment),
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

	std::vector<vk::AttachmentDescription> attachments = {
			colorAttachment,
			depthAttachment
	};

	vk::RenderPassCreateInfo rpci(
			{},
			attachments,
			subpass,
			sd);

	renderPass = device.createRenderPass(rpci);
}

void Application::createDescriptorSetLayout(bool verbose)
{
	const vk::raii::Device& device = environment.device();

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
	scenewise_descriptor_set_layout = device.createDescriptorSetLayout(scenewise_layout_ci);

	vk::DescriptorSetLayoutCreateInfo objectwise_layout_ci({}, objectwise_layout);
	objectwise_descriptor_set_layout = device.createDescriptorSetLayout(objectwise_layout_ci);
}

void Application::createGraphicsPipeline(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::raii::ShaderModule vert = createShader(environment, vertCode);
	vk::raii::ShaderModule frag = createShader(environment, fragCode);

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


	std::vector<vk::DescriptorSetLayout> layouts = {
			*scenewise_descriptor_set_layout,
			*objectwise_descriptor_set_layout,
	};
	vk::PipelineLayoutCreateInfo plci({}, layouts);

	pipeline_layout = device.createPipelineLayout(plci);

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
			*renderPass,
			0);

	pipelines = device.createGraphicsPipelines(nullptr, gpci);
}

void Application::createFramebuffers(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	framebuffers.clear();
	framebuffers.reserve(imageViews.size());
	for (auto& imageView: imageViews)
	{
		vk::ImageView attachments[] = {
				*imageView,
				*depthImageView,
		};

		vk::FramebufferCreateInfo fci({}, *renderPass, attachments, scImageExtent.width, scImageExtent.height, 1);
		framebuffers.push_back(device.createFramebuffer(fci));
	}
}

void Application::createCommandPool(bool verbose)
{
	const int graphics_queue_family = environment.graphics_queue_index();
	const vk::raii::Device& device = environment.device();

	vk::CommandPoolCreateInfo cpci(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphics_queue_family);

	commandPool = device.createCommandPool(cpci);
}

void Application::createDepthResources(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::Format format = findDepthFormat(environment);
//	auto&& images = createImage(environment,
//			scImageExtent.width, scImageExtent.height, 1, format, vk::ImageTiling::eOptimal,
//			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
//	depthImage = std::move(images.first);
//	depthImageMemory = std::move(images.second);

	std::tie(depthImage, depthImageMemory) = createImage(environment,
			scImageExtent.width, scImageExtent.height, 1, format, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::ImageViewCreateInfo ivci(
			{},
			*depthImage,
			vk::ImageViewType::e2D,
			format,
			{},
			{
					vk::ImageAspectFlagBits::eDepth,
					0,
					1,
					0,
					1 });
	depthImageView = device.createImageView(ivci);

	transitionImageLayout(
			environment,
			commandPool,
			*depthImage,
			format,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			1);
}

void Application::createTextureImage(const asset::image& image, engine::texture& res, vk::Format format)
{
//	int texW, texH, texChannels;
//	stbi_uc* pixels = stbi_load(name.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
//	vk::DeviceSize imageSize = texW * texH * 4;
//	res.texture.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;
//
//	if (!pixels)
//	{
//		throw std::runtime_error("failed to load image");
//	}

	const vk::DeviceSize image_size = image.width() * image.height() * 4;
	const uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width(), image.height())))) + 1;

	vk::raii::Buffer staging_buffer = nullptr;
	vk::raii::DeviceMemory staging_memory = nullptr;
	std::tie(staging_buffer, staging_memory) = vkh::staging_buffer(environment, image_size);
//	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
//			imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
//			                                                  | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = staging_memory.mapMemory(0, image_size);
	std::memcpy(data, image.data().data(), image_size);
	staging_memory.unmapMemory();


	vk::raii::Image texture_image = nullptr;
	vk::raii::DeviceMemory texture_memory = nullptr;
	std::tie(texture_image, texture_memory) = vkh::image(
			environment,
			image.width(),
			image.height(),
			mip_levels,
			format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc
			| vk::ImageUsageFlagBits::eTransferDst
			| vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
//	auto&& tmp = createImage(
//			texW, texH, res.texture.mip_levels, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
//			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
//			| vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
//	res.texture.image = std::move(tmp.first);
//	res.texture.mem = std::move(tmp.second);

	vkh::transition_image_layout(
			environment,
			commandPool,
			texture_image,
			format,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			mip_levels);
	vkh::copy_buffer_to_image(
			environment,
			commandPool,
			staging_buffer,
			texture_image,
			image.width(),
			image.height());
	vkh::generate_mipmaps(
			environment,
			commandPool,
			texture_image,
			format,
			image.width(),
			image.height(),
			mip_levels);
//	vkh::transition_image_layout(
//			environment,
//			commandPool,
//			texture_image,
//			format,
//			vk::ImageLayout::eTransferDstOptimal,
//			vk::ImageLayout::eShaderReadOnlyOptimal,
//			mip_levels);

	res.image = std::move(texture_image);
	res.mip_levels = mip_levels;
	res.mem = std::move(texture_memory);

//	transitionImageLayout(
//			*res.texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
//			res.texture.mip_levels);
//	copyBufferToImage(*stagingBuffer, *res.texture.image, texW, texH);
//	transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
//	generateMipmaps(*res.texture.image, vk::Format::eR8G8B8A8Srgb, texW, texH, res.texture.mip_levels);
}

void Application::createTextureImageView(engine::texture& res, vk::Format format)
{
	const vk::raii::Device& device = environment.device();
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
	res.image_view = device.createImageView(ivci);
}

void Application::createTextureSampler(engine::texture& res)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();
	const vk::raii::Device& device = environment.device();

	vk::PhysicalDeviceProperties properties = phys_device.getProperties();

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

	res.sampler = device.createSampler(sci);
}


void Application::loadModel(const std::string& name, engine::scene::prop& res, bool verbose)
{
	mesh::vertex v;
	v.position = glm::vec3(-1.0f, -1.0f, 0.0f);
	v.texcoord = glm::vec3(0.0f, 0.0f, 0.0f);
	v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
	res.mesh.vertices.push_back(v);
	v.position = glm::vec3(1.0f, -1.0f, 0.0f);
	v.texcoord = glm::vec3(2.0f, 0.0f, 0.0f);
	res.mesh.vertices.push_back(v);
	v.position = glm::vec3(1.0f, 1.0f, 0.0f);
	v.texcoord = glm::vec3(2.0f, 2.0f, 0.0f);
	res.mesh.vertices.push_back(v);
	v.position = glm::vec3(-1.0f, 1.0f, 0.0f);
	v.texcoord = glm::vec3(0.0f, 2.0f, 0.0f);
	res.mesh.vertices.push_back(v);
	res.mesh.indices = {
			0, 1, 2,
			0, 2, 3,
	};
	calculate_tangent_space(res.mesh);
	return;

	geom::obj obj = geom::obj::load_obj(name);
//	obj = geom::triangulate_obj(obj);

	std::map<std::pair<int32_t, int32_t>, int32_t> vertices;

	std::vector<std::set<int>> vertex_to_triangle(obj.n_vertices());

	for (const geom::obj::index_triplet& t : obj.face_indices())
	{
		std::pair<int32_t, int32_t> key = { t.vi, t.vti };

		const auto& thing = vertices.find(key);

		if (thing == vertices.end())
		{
			mesh::vertex v{};
			v.position = obj.vertices()[t.vi];
			v.texcoord = obj.texcoords()[t.vti];
			v.normal = obj.normals()[t.vni];
//			v.texcoord.y = 1.0f - v.texcoord.y;

			vertices.insert_or_assign(key, res.mesh.vertices.size());
			res.mesh.vertices.push_back(v);
		}
		res.mesh.indices.push_back(vertices.at(key));
		vertex_to_triangle[t.vi].insert(vertices.at(key));
	}

	std::vector<std::vector<int>> triangle_to_vertex(obj.n_face_indices());
	for (int i = 0; i < vertex_to_triangle.size(); i++)
	{
		for (auto iter = vertex_to_triangle[i].begin(); iter != vertex_to_triangle[i].end(); iter++)
		{
			for (auto inter = vertex_to_triangle[i].begin(); inter != vertex_to_triangle[i].end(); inter++)
			{
				triangle_to_vertex[*iter].push_back(*inter);
			}
		}
	}
	calculate_tangent_space(res.mesh);
}

void Application::createVertexBuffers(engine::scene::prop& res, bool verbose)
{
	const vk::DeviceSize size = sizeof(res.mesh.vertices[0]) * res.mesh.vertices.size();

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			environment,
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.vertices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			environment,
			size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.geom_buffer.vertex_buffer = std::move(buffers.first);
	res.geom_buffer.vertex_buffer_memory = std::move(buffers.second);

	copyBuffer(
			environment,
			commandPool,
			stagingBuffer,
			res.geom_buffer.vertex_buffer,
			size);
}

void Application::createIndexBuffers(engine::scene::prop& res, bool verbose)
{
	const vk::DeviceSize size = sizeof(res.mesh.indices[0]) * res.mesh.indices.size();

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			environment,
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.indices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			environment,
			size,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	res.geom_buffer.index_buffer = std::move(buffers.first);
	res.geom_buffer.index_buffer_memory = std::move(buffers.second);

	copyBuffer(
			environment,
			commandPool,
			stagingBuffer,
			res.geom_buffer.index_buffer,
			size);
}

void Application::createUniformBuffers(bool verbose)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();

	// view_projection
	{
		const vk::DeviceSize size = sizeof(view_projection);

		auto&& buffers = createBuffer(
				environment,
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		view_projection_buffer = std::move(buffers.first);
		view_projection_buffer_memory = std::move(buffers.second);
	}

	// light_sources
	{
		const vk::DeviceSize alignment = phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
		const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
		const vk::DeviceSize size = padded_size + sizeof(engine::lights::spot_lights_t);

		auto&& buffers = createBuffer(
				environment,
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		light_sources_buffer = std::move(buffers.first);
		light_sources_buffer_memory = std::move(buffers.second);
	}

	// models
	const vk::DeviceSize size = props.size() * sizeof(glm::mat4);

	auto&& buffers = createBuffer(
			environment,
			size,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	models_buffer = std::move(buffers.first);
	models_buffer_memory = std::move(buffers.second);
}

void Application::createDescriptorPool(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	std::vector<vk::DescriptorPoolSize> poolSizes = {
			{ vk::DescriptorType::eUniformBuffer,        (uint32_t)(3 * maxFrames + maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
			{ vk::DescriptorType::eCombinedImageSampler, (uint32_t)(maxFrames * props.size())},
	};

	vk::DescriptorPoolCreateInfo dpci(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxFrames * (1 + props.size()), poolSizes);

	descriptorPool = device.createDescriptorPool(dpci);
}

void Application::createImguiDescriptorPool(bool verbose)
{
	const vk::raii::Device& device = environment.device();

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

	ImguiDescriptorPool = device.createDescriptorPool(dpci);
}

void Application::createDescriptorSets(bool verbose)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();
	const vk::raii::Device& device = environment.device();

	std::vector<vk::DescriptorSetLayout> counts(maxFrames, *scenewise_descriptor_set_layout);
	vk::DescriptorSetAllocateInfo dsai(*descriptorPool, counts);

	scenewise_descriptor_sets = device.allocateDescriptorSets(dsai);

	counts = std::vector<vk::DescriptorSetLayout>(maxFrames * props.size(), *objectwise_descriptor_set_layout);
	dsai = vk::DescriptorSetAllocateInfo(*descriptorPool, counts);

	objectwise_descriptor_sets = device.allocateDescriptorSets(dsai);

	vk::DescriptorBufferInfo view_projection_info(*view_projection_buffer, 0, sizeof(view_projection));
	vk::DescriptorBufferInfo point_lights_info(*light_sources_buffer, 0, sizeof(engine::lights::point_lights_t));

	const vk::DeviceSize alignment = phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
	const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
	vk::DescriptorBufferInfo spot_lights_info(*light_sources_buffer, padded_size, sizeof(engine::lights::spot_lights_t));

	std::vector<vk::WriteDescriptorSet> writes = {
			{*scenewise_descriptor_sets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, view_projection_info},
			{*scenewise_descriptor_sets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, point_lights_info},
			{*scenewise_descriptor_sets[0], 2, 0, vk::DescriptorType::eUniformBuffer, nullptr, spot_lights_info},

			{*scenewise_descriptor_sets[1], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, view_projection_info},
			{*scenewise_descriptor_sets[1], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, point_lights_info},
			{*scenewise_descriptor_sets[1], 2, 0, vk::DescriptorType::eUniformBuffer, nullptr, spot_lights_info},
	};

	device.updateDescriptorSets(writes, {});

	for (int i = 0; i < props.size(); i++)
	{
		vk::DescriptorBufferInfo model_info(*models_buffer, i * sizeof(glm::mat4), sizeof(glm::mat4));

		vk::DescriptorImageInfo texture_info(*props[i].texture.sampler, *props[i].texture.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo normal_info(*props[i].normal.sampler, *props[i].normal.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo roughness_info(*props[i].roughness.sampler, *props[i].roughness.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo metallic_info(*props[i].metallic.sampler, *props[i].metallic.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::DescriptorImageInfo ao_info(*props[i].ambient_occlusion.sampler, *props[i].ambient_occlusion.image_view, vk::ImageLayout::eShaderReadOnlyOptimal);

		std::vector<vk::WriteDescriptorSet> descriptorWrites = {
				{*objectwise_descriptor_sets[2 * i + 0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, model_info},
				{*objectwise_descriptor_sets[2 * i + 0], 1, 0, vk::DescriptorType::eCombinedImageSampler, texture_info},
				{*objectwise_descriptor_sets[2 * i + 0], 2, 0, vk::DescriptorType::eCombinedImageSampler, normal_info},
				{*objectwise_descriptor_sets[2 * i + 0], 3, 0, vk::DescriptorType::eCombinedImageSampler, roughness_info},
				{*objectwise_descriptor_sets[2 * i + 0], 4, 0, vk::DescriptorType::eCombinedImageSampler, metallic_info},
				{*objectwise_descriptor_sets[2 * i + 0], 5, 0, vk::DescriptorType::eCombinedImageSampler, ao_info},

				{*objectwise_descriptor_sets[2 * i + 1], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, model_info},
				{*objectwise_descriptor_sets[2 * i + 1], 1, 0, vk::DescriptorType::eCombinedImageSampler, texture_info},
				{*objectwise_descriptor_sets[2 * i + 1], 2, 0, vk::DescriptorType::eCombinedImageSampler, normal_info},
				{*objectwise_descriptor_sets[2 * i + 1], 3, 0, vk::DescriptorType::eCombinedImageSampler, roughness_info},
				{*objectwise_descriptor_sets[2 * i + 1], 4, 0, vk::DescriptorType::eCombinedImageSampler, metallic_info},
				{*objectwise_descriptor_sets[2 * i + 1], 5, 0, vk::DescriptorType::eCombinedImageSampler, ao_info},
		};

		device.updateDescriptorSets(descriptorWrites, {});
	}
}

void Application::createCommandBuffers(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::CommandBufferAllocateInfo cbai(*commandPool, vk::CommandBufferLevel::ePrimary, maxFrames);
	commandBuffers = device.allocateCommandBuffers(cbai);
}

void Application::createSyncObjects(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::SemaphoreCreateInfo sci;

	vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);

	for (int i = 0; i < maxFrames; i++)
	{
		imageAvailableSemaphores.push_back(device.createSemaphore(sci));
		renderFinishedSemaphores.push_back(device.createSemaphore(sci));
		isFlightFences.push_back(device.createFence(fci));

	}
}

void Application::recreateSwapChain(bool verbose)
{
	const vk::raii::Device& device = environment.device();

	vk::Extent2D ext = window.framebuffer_size();
	while (ext.height == 0 || ext.width == 0)
	{
		ext = window.framebuffer_size();
		glfw::window::wait_events();
	}

	device.waitIdle();

	framebuffers.clear();
	imageViews.clear();
	swapchain = nullptr;

	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();
}


void Application::drawFrame()
{
	const vk::raii::Device& device = environment.device();
	const vk::raii::Queue& graphics_queue = environment.graphics_queue();
	const vk::raii::Queue& present_queue = environment.present_queue();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (isMenuShowing)
		ImGui::ShowDemoWindow();

	ImGui::Render();

	glfw::key_action action = window.get_key(glfw::key_id::A);
	if (action == glfw::key_action::press)
	{
		cam.position -= cam.right / 30.0f;
	}
	action = window.get_key(glfw::key_id::D);
	if (action == glfw::key_action::press)
	{
		cam.position += cam.right / 30.0f;
	}
	action = window.get_key(glfw::key_id::W);
	if (action == glfw::key_action::press)
	{
		cam.position += cam.fwd / 30.0f;
	}
	action = window.get_key(glfw::key_id::S);
	if (action == glfw::key_action::press)
	{
		cam.position -= cam.fwd / 30.0f;
	}
	action = window.get_key(glfw::key_id::escape);
	if (action == glfw::key_action::press)
	{
		if (!lockChange && !isMenuShowing)
		{
			window.set_input_mode(GLFW_CURSOR_NORMAL);
			isMenuShowing = true;
		}
		else if (!lockChange)
		{
			window.set_input_mode(GLFW_CURSOR_DISABLED);
			isMenuShowing = false;
		}
		lockChange = true;
	}
	if (action == glfw::key_action::release)
	{
		lockChange = false;
	}

	vk::Result result = device.waitForFences(*isFlightFences[currentFrame], true, UINT64_MAX);
	resultCheck(result, "Fence checker");

	uint32_t imgIndex;
	vk::AcquireNextImageInfoKHR anii(*swapchain, UINT64_MAX, *imageAvailableSemaphores[currentFrame], {}, 1);
	auto tmp = device.acquireNextImage2KHR(anii);
	result = tmp.first;
	if (result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
	{
		frameBufferResized = false;
		recreateSwapChain();
		return;
	}
	device.resetFences(*isFlightFences[currentFrame]);
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

	graphics_queue.submit(si, *isFlightFences[currentFrame]);


	vk::PresentInfoKHR pi(*renderFinishedSemaphores[currentFrame], *swapchain, imgIndex);

	try
	{
		present_queue.presentKHR(pi);
		currentFrame = (currentFrame + 1) % maxFrames;
	}
	catch (...)
	{
		frameBufferResized = false;
		recreateSwapChain();
		currentFrame = (currentFrame + 1) % maxFrames;
	}

	//currentFrame = (currentFrame + 1) % maxFrames;
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
			*renderPass,
			*framebuffers[imgIndex],
			{{ 0, 0 }, scImageExtent },
			clearColors);
	buffer.beginRenderPass(rpbi, vk::SubpassContents::eInline);
	{
		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipelines[0]);
		vk::Viewport vp(
				0.0f, 0.0f,
				static_cast<float>(scImageExtent.width), static_cast<float>(scImageExtent.height), 0.0f,
				1.0f);
		buffer.setViewport(0, vp);

		vk::Rect2D sc({ 0, 0 }, scImageExtent);
		buffer.setScissor(0, sc);

		buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, *scenewise_descriptor_sets[currentFrame], {});

		for (int i = 0; i < props.size(); i++)
		{
			std::vector<vk::DeviceSize> ds = { 0 };
			buffer.bindVertexBuffers(0, *props[i].geom_buffer.vertex_buffer, ds);
			buffer.bindIndexBuffer(*props[i].geom_buffer.index_buffer, 0, vk::IndexType::eUint32);

			buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout, 1, *objectwise_descriptor_sets[2 * i + currentFrame], {});

			buffer.drawIndexed(props[i].mesh.indices.size(), 1, 0, 0, 0);
		}

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *buffer);
	}
	buffer.endRenderPass();
	buffer.end();
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


	view_proj.view = cam.view();
	view_proj.projection = glm::perspective(
			glm::radians(60.0f), scImageExtent.width / float(scImageExtent.height), 0.1f, 400.0f);
	view_proj.projection[1][1] *= -1;
	view_proj.camPos = cam.position;

	glm::mat4 r = glm::mat4(1.0f);
	r = glm::rotate(r, glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	scene.lights.point_lights.lights[0].position = r * scene.lights.point_lights.lights[0].position;

	scene.lights.spot_lights.lights[0].position = glm::vec4(cam.position, 0.0f);
	scene.lights.spot_lights.lights[0].direction = glm::normalize(glm::vec4(cam.fwd, 0.0f));

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

		const vk::DeviceSize alignment = phys_device.getProperties().limits.minUniformBufferOffsetAlignment;
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
	const vk::raii::Device& device = environment.device();

	//window.set_close(true);
	while (!window.should_close())
	{
		glfw::window::poll_events();
		drawFrame();
	}

	device.waitIdle();
}


vk::raii::PhysicalDevice
Application::pickDevice(const std::vector<vk::raii::PhysicalDevice>& devices)
{
	if (devices.empty())
	{
		throw std::runtime_error("No GPUs present!");
	}

	return devices[0];
}


Application::SwapChainSupportDetails
Application::querySupport(const vk_utils::environment& environment)
{
	Application::SwapChainSupportDetails details{};

	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();
	const vk::raii::SurfaceKHR& surface = environment.surface(0);

	details.capabilities = phys_device.getSurfaceCapabilitiesKHR(*surface);
	details.formats = phys_device.getSurfaceFormatsKHR(*surface);
	details.presentModes = phys_device.getSurfacePresentModesKHR(*surface);


	return details;
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

	if (std::find(details.presentModes.begin(), details.presentModes.end(), vk::PresentModeKHR::eMailbox)
	    != details.presentModes.end())
	{
		return vk::PresentModeKHR::eMailbox;
	}

}


uint32_t
Application::findMemoryType(const vk_utils::environment& environment, uint32_t filter, vk::MemoryPropertyFlags properties)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();

	vk::PhysicalDeviceMemoryProperties mp = phys_device.getMemoryProperties();
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
		const vk_utils::environment& environment,
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();

	for (vk::Format format: candidates)
	{
		vk::FormatProperties props = phys_device.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features
		    || tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

vk::Format Application::findDepthFormat(const vk_utils::environment& environment)
{
	return findSupportedFormat(
			environment,
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}


vk::raii::ShaderModule
Application::createShader(const vk_utils::environment& environment, const std::vector<uint32_t>& code)
{
	const vk::raii::Device& device = environment.device();

	vk::ShaderModuleCreateInfo smci(
			{},
			4 * code.size(),
			code.data());

	return device.createShaderModule(smci);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Application::createBuffer(const vk_utils::environment& environment, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	const vk::raii::Device& device = environment.device();

	vk::raii::Buffer resBuf = nullptr;
	vk::raii::DeviceMemory resBufMem = nullptr;
	vk::BufferCreateInfo bci({}, size, usage, vk::SharingMode::eExclusive);
	resBuf = vk::raii::Buffer(device, bci);

	vk::MemoryRequirements mr = resBuf.getMemoryRequirements();

	vk::MemoryAllocateInfo mai(mr.size, findMemoryType(environment, mr.memoryTypeBits, properties));
	resBufMem = device.allocateMemory(mai);

	resBuf.bindMemory(*resBufMem, 0);

	return { std::move(resBuf), std::move(resBufMem) };
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory>
Application::createImage(
		const vk_utils::environment& environment,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties)
{
	const vk::raii::Device& device = environment.device();

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
	img = device.createImage(ici);

	vk::DeviceImageMemoryRequirements dimr(&ici);
	vk::MemoryRequirements2 memRequirements = device.getImageMemoryRequirements(dimr);

	vk::MemoryAllocateInfo allocInfo(
			memRequirements.memoryRequirements.size,
			findMemoryType(
					environment,
					memRequirements.memoryRequirements.memoryTypeBits,
					properties));
	mem = device.allocateMemory(allocInfo);

	vk::BindImageMemoryInfo bimi(*img, *mem, 0);
	device.bindImageMemory2(bimi);

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


vk::raii::CommandBuffer
Application::beginSingleTimeCommands(const vk_utils::environment& environment, const vk::raii::CommandPool& cmd_pool)
{
	const vk::raii::Device& device = environment.device();

	vk::CommandBufferAllocateInfo cbai(*cmd_pool, vk::CommandBufferLevel::ePrimary, 1);

	vk::raii::CommandBuffer cb = std::move(device.allocateCommandBuffers(cbai)[0]);

	vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cb.begin(cbbi);

	return cb;
}

void
Application::endSingleTimeCommands(const vk_utils::environment& environment, const vk::raii::CommandBuffer& buffer)
{
	const vk::raii::Queue& graphics_queue = environment.graphics_queue();
	buffer.end();

	vk::SubmitInfo si({}, {}, *buffer);
	graphics_queue.submit(si);
	graphics_queue.waitIdle();
}


void
Application::copyBuffer(const vk_utils::environment& environment, const vk::raii::CommandPool& cmd_pool, const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands(environment, cmd_pool);

	vk::BufferCopy bc(0, 0, size);
	cb.copyBuffer(*srcBuffer, *dstBuffer, bc);

	endSingleTimeCommands(environment, cb);
}

void
Application::copyBufferToImage(
		const vk_utils::environment& environment,
		const vk::raii::CommandPool& cmd_pool,
		vk::Buffer buffer,
		vk::Image image,
		uint32_t width,
		uint32_t height)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands(environment, cmd_pool);

	vk::BufferImageCopy bic(
			0, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, { width, height, 1 });

	cb.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, bic);

	endSingleTimeCommands(environment, cb);
}


void
Application::transitionImageLayout(
		const vk_utils::environment& environment,
		const vk::raii::CommandPool& cmd_pool,
		vk::Image image,
		vk::Format format,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t mipLevels)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands(environment, cmd_pool);

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

	endSingleTimeCommands(environment, cb);
}

void
Application::generateMipmaps(
		const vk_utils::environment& environment,
		const vk::raii::CommandPool& cmd_pool,
		vk::Image img,
		vk::Format imageFormat,
		int32_t texW,
		int32_t texH,
		uint32_t mipLevels)
{
	const vk::raii::PhysicalDevice& phys_device = environment.phys_device();

	vk::FormatProperties propertiess = phys_device.getFormatProperties(imageFormat);
	if (!(propertiess.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	vk::raii::CommandBuffer buffer = beginSingleTimeCommands(environment, cmd_pool);

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

	endSingleTimeCommands(environment, buffer);
}


bool Application::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}


} // gorilla