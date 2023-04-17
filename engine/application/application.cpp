#include <iostream>

#include "application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <obj/obj.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <set>


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
	initEnvironment(false);

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
		props.push_back({});
		createTextureImage(scene.props[i].texture_filename, props.back());
		createTextureImageView(props.back());
		createTextureSampler(props.back());

		createNormalImage(scene.props[i].normal_filename, props.back());
		createNormalImageView(props.back());
		createNormalSampler(props.back());

		createRoughnessImage(scene.props[i].roughness_filename, props.back());
		createRoughnessImageView(props.back());
		createRoughnessSampler(props.back());

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
	i.Instance = *instance;
	i.PhysicalDevice = *physDevice;
	i.Device = *device;
	i.Queue = *graphicsQueue;
	i.QueueFamily = graphicsQueueFamilyIndex;
	i.DescriptorPool = *ImguiDescriptorPool;
	i.ImageCount = maxFrames;
	i.MinImageCount = maxFrames;
	i.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//	ImGui_ImplVulkan_Init(&i, *renderPass);
	imgui = std::make_unique<engine::misc::imgui_object>(window.handle(), true, &i,
	                                   *renderPass);

	auto cmd = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(*cmd);
	endSingleTimeCommands(*cmd);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void
Application::initEnvironment(bool verbose)
{
	establishContext(verbose);
	createInstance(verbose);
	createSurface(verbose);
	createPhysDevice(verbose);
	createLogicalDevice(verbose);
	createQueue(verbose);
}

void
Application::establishContext(bool verbose)
{
	instanceVersion = context.enumerateInstanceVersion();
	layerProperties = context.enumerateInstanceLayerProperties();
	extensionProperties = context.enumerateInstanceExtensionProperties();

	if (!verbose)
	{
		return;
	}


	std::cout << "Instance Version: ["
	          << VK_VERSION_MAJOR(instanceVersion) << "."
	          << VK_VERSION_MINOR(instanceVersion) << "."
	          << VK_VERSION_PATCH(instanceVersion) << "]"
	          << std::endl;

	std::cout << std::endl
	          << "Layer Properties:"
	          << std::endl;
	int i = 0;
	for (const auto& layer: layerProperties)
	{
		std::cout << " " << ++i << ". "
		          << layer.layerName << " ["
		          << VK_VERSION_MAJOR(layer.specVersion) << "."
		          << VK_VERSION_MINOR(layer.specVersion) << "."
		          << VK_VERSION_PATCH(layer.specVersion) << ":"
		          << layer.implementationVersion << "]"
		          << std::endl;
		std::cout << "    " << layer.description
		          << std::endl;
	}

	std::cout << std::endl
	          << "Extension Properties:"
	          << std::endl;
	i = 0;
	for (const auto& ext: extensionProperties)
	{
		std::cout << " " << ++i << ". "
		          << ext.extensionName << " ["
		          << ext.specVersion << "]"
		          << std::endl;
	}
}

void
Application::createInstance(bool verbose)
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
	instance = vk::raii::Instance(context, ici);
}

void
Application::createSurface(bool verbose)
{
	surface = window.surface(instance);
}

void
Application::createPhysDevice(bool verbose)
{
	physDevice = pickDevice(instance.enumeratePhysicalDevices());
	queueFamilies = physDevice.getQueueFamilyProperties();
	deviceLayerProperties = physDevice.enumerateDeviceLayerProperties();
	deviceExtensionProperties = physDevice.enumerateDeviceExtensionProperties();

	for (int i = 0; i < queueFamilies.size(); ++i)
	{
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsQueueFamilyIndex = i;
		}

		if (physDevice.getSurfaceSupportKHR(i, *surface))
		{
			presentQueueFamilyIndex = i;
		}
	}

	if (!verbose)
	{
		return;
	}

	std::cout << std::endl
	          << "Queue Families:"
	          << std::endl;
	int i = 0;
	for (const auto& family: queueFamilies)
	{
		std::cout << " " << ++i << ". ";
		if (family.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			std::cout << "Graphics ";
		}
		if (family.queueFlags & vk::QueueFlagBits::eCompute)
		{
			std::cout << "Compute ";
		}
		if (family.queueFlags & vk::QueueFlagBits::eProtected)
		{
			std::cout << "Protected ";
		}
		if (family.queueFlags & vk::QueueFlagBits::eSparseBinding)
		{
			std::cout << "SparseBinding ";
		}
		if (family.queueFlags & vk::QueueFlagBits::eTransfer)
		{
			std::cout << "Transfer ";
		}

		std::cout << "["
		          << family.queueCount
		          << "]"
				  << std::endl;
	}

	std::cout << std::endl
	          << "Device Layer Properties:"
	          << std::endl;
	i = 0;
	for (const auto& layer: deviceLayerProperties)
	{
		std::cout << " " << ++i << ". "
		          << layer.layerName << " ["
		          << VK_VERSION_MAJOR(layer.specVersion) << "."
		          << VK_VERSION_MINOR(layer.specVersion) << "."
		          << VK_VERSION_PATCH(layer.specVersion) << ":"
		          << layer.implementationVersion << "]"
		          << std::endl;
		std::cout << "    " << layer.description
		          << std::endl;
	}

	std::cout << std::endl
	          << "Device Extension Properties:"
	          << std::endl;
	i = 0;
	for (const auto& ext: deviceExtensionProperties)
	{
		std::cout << " " << ++i << ". "
		          << ext.extensionName << " ["
		          << ext.specVersion << "]"
		          << std::endl;
	}
}

vk::raii::PhysicalDevice
Application::pickDevice(const std::vector<vk::raii::PhysicalDevice>& devices) const
{
	if (devices.empty())
	{
		throw std::runtime_error("No GPUs present!");
	}

	return devices[0];
}

void
Application::createLogicalDevice(bool verbose)
{
	const std::array<float, 1> priorities = { 1.0f };

	vk::DeviceQueueCreateInfo dqci({}, graphicsQueueFamilyIndex, priorities);
	vk::DeviceQueueCreateInfo dqci2({}, presentQueueFamilyIndex, priorities);

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

	device = vk::raii::Device(physDevice, dci);
}

void
Application::createQueue(bool verbose)
{
	graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);
	presentQueue = device.getQueue(presentQueueFamilyIndex, 0);
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
	Application::SwapChainSupportDetails details = querySupport(physDevice);
	vk::Format format;
	vk::ColorSpaceKHR colorSpace;
	pickBestFormat(details, format, colorSpace);
	vk::PresentModeKHR presentMode = pickPresentMode(details);

	vk::Extent2D extent = window.framebuffer_size();
	vk::Extent2D actualExtent = extent;
	actualExtent.width = std::clamp(
			actualExtent.width, details.capabilities.minImageExtent.width,
			details.capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(
			actualExtent.height, details.capabilities.minImageExtent.height,
			details.capabilities.maxImageExtent.height);

	std::array<uint32_t, 2> bl = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
	vk::SwapchainCreateInfoKHR scci(
			{},
			*surface,
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
	swapchain = vk::raii::SwapchainKHR(device, scci);
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
		imageViews.emplace_back(device, ivci);
	}
}


Application::SwapChainSupportDetails Application::querySupport(const vk::raii::PhysicalDevice& physicalDevice) const
{
	Application::SwapChainSupportDetails details{};

	details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	details.formats = physicalDevice.getSurfaceFormatsKHR(*surface);
	details.presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

	return details;
}

void Application::pickBestFormat(
		const Application::SwapChainSupportDetails& details, vk::Format& outFormat,
		vk::ColorSpaceKHR& outColorSpace) const
{
	outFormat = details.formats[1].format;
	outColorSpace = details.formats[1].colorSpace;
}

vk::PresentModeKHR Application::pickPresentMode(const Application::SwapChainSupportDetails& details) const
{
	return vk::PresentModeKHR::eFifo;

	if (std::find(details.presentModes.begin(), details.presentModes.end(), vk::PresentModeKHR::eMailbox)
	    != details.presentModes.end())
	{
		return vk::PresentModeKHR::eMailbox;
	}

}

void Application::recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const
{
	vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	buffer.begin(cbbi);

	std::array<float, 4> clr = { 0.6f, 0.6f, 0.6f, 1.0f };
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

uint32_t Application::findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties mp = physDevice.getMemoryProperties();
	for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
	{
		if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Nu suitable memory type found!");
}

std::vector<char> Application::readFile(const std::string& filename)
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

vk::raii::ShaderModule Application::createShader(const std::vector<uint32_t>& code)
{
	vk::ShaderModuleCreateInfo smci(
			{},
			4 * code.size(),
			code.data());

	return device.createShaderModule(smci);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
Application::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	vk::raii::Buffer resBuf = nullptr;
	vk::raii::DeviceMemory resBufMem = nullptr;
	vk::BufferCreateInfo bci({}, size, usage, vk::SharingMode::eExclusive);
	resBuf = vk::raii::Buffer(device, bci);

	vk::MemoryRequirements mr = resBuf.getMemoryRequirements();

	vk::MemoryAllocateInfo mai(mr.size, findMemoryType(mr.memoryTypeBits, properties));
	resBufMem = device.allocateMemory(mai);

	resBuf.bindMemory(*resBufMem, 0);

	return { std::move(resBuf), std::move(resBufMem) };
}

void
Application::copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands();

	vk::BufferCopy bc(0, 0, size);
	cb.copyBuffer(*srcBuffer, *dstBuffer, bc);

	endSingleTimeCommands(*cb);
}

void Application::updateUniformBuffer(uint32_t currentImage)
{
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

		const vk::DeviceSize alignment = physDevice.getProperties().limits.minUniformBufferOffsetAlignment;
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

std::pair<vk::raii::Image, vk::raii::DeviceMemory> Application::createImage(
		uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
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
	img = device.createImage(ici);

	vk::DeviceImageMemoryRequirements dimr(&ici);
	vk::MemoryRequirements2 memRequirements = device.getImageMemoryRequirements(dimr);

	vk::MemoryAllocateInfo allocInfo(
			memRequirements.memoryRequirements.size,
			findMemoryType(memRequirements.memoryRequirements.memoryTypeBits, properties));
	mem = device.allocateMemory(allocInfo);

	vk::BindImageMemoryInfo bimi(*img, *mem, 0);
	device.bindImageMemory2(bimi);

	return { std::move(img), std::move(mem) };
}

vk::raii::CommandBuffer Application::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo cbai(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

	vk::raii::CommandBuffer cb = std::move(device.allocateCommandBuffers(cbai)[0]);

	vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cb.begin(cbbi);

	return cb;
}

void Application::endSingleTimeCommands(vk::CommandBuffer buffer)
{
	buffer.end();

	vk::SubmitInfo si({}, {}, buffer);
	graphicsQueue.submit(si);
	graphicsQueue.waitIdle();
}

void Application::transitionImageLayout(
		vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
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

	endSingleTimeCommands(*cb);
}

void Application::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
	vk::raii::CommandBuffer cb = beginSingleTimeCommands();

	vk::BufferImageCopy bic(
			0, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, { width, height, 1 });

	cb.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, bic);

	endSingleTimeCommands(*cb);
}

vk::Format Application::findSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (vk::Format format: candidates)
	{
		vk::FormatProperties props = physDevice.getFormatProperties(format);

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

bool Application::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void
Application::generateMipmaps(vk::Image img, vk::Format imageFormat, int32_t texW, int32_t texH, uint32_t mipLevels)
{
	vk::FormatProperties props = physDevice.getFormatProperties(imageFormat);
	if (!(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
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

	endSingleTimeCommands(*buffer);
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
	};

	vk::DescriptorSetLayoutCreateInfo scenewise_layout_ci({}, scenewise_layout);
	scenewise_descriptor_set_layout = device.createDescriptorSetLayout(scenewise_layout_ci);

	vk::DescriptorSetLayoutCreateInfo objectwise_layout_ci({}, objectwise_layout);
	objectwise_descriptor_set_layout = device.createDescriptorSetLayout(objectwise_layout_ci);
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
	vk::CommandPoolCreateInfo cpci(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueFamilyIndex);

	commandPool = device.createCommandPool(cpci);
}

void Application::createDepthResources(bool verbose)
{
	vk::Format format = findDepthFormat();
	auto&& images = createImage(
			scImageExtent.width, scImageExtent.height, 1, format, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
	depthImage = std::move(images.first);
	depthImageMemory = std::move(images.second);

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
			*depthImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
}

void Application::createTextureImage(const std::string& name, engine::scene::prop& res, bool verbose)
{
	int texW, texH, texChannels;
	stbi_uc* pixels = stbi_load(name.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = texW * texH * 4;
	res.texture.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;

	if (!pixels)
	{
		throw std::runtime_error("failed to load image");
	}

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                                  | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = stagingBufferMemory.mapMemory(0, imageSize);
	std::memcpy(data, pixels, imageSize);
	stagingBufferMemory.unmapMemory();

	stbi_image_free(pixels);

	auto&& tmp = createImage(
			texW, texH, res.texture.mip_levels, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
			| vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.texture.image = std::move(tmp.first);
	res.texture.mem = std::move(tmp.second);

	transitionImageLayout(
			*res.texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			res.texture.mip_levels);
	copyBufferToImage(*stagingBuffer, *res.texture.image, texW, texH);
//	transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
	generateMipmaps(*res.texture.image, vk::Format::eR8G8B8A8Srgb, texW, texH, res.texture.mip_levels);
}

void Application::createTextureImageView(engine::scene::prop& res, bool verbose)
{
	vk::ImageViewCreateInfo ivci(
			{},
			*res.texture.image,
			vk::ImageViewType::e2D,
			vk::Format::eR8G8B8A8Srgb,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					res.texture.mip_levels,
					0,
					1));
	res.texture.image_view = { device, ivci };
}

void Application::createTextureSampler(engine::scene::prop& res, bool verbose)
{
	vk::PhysicalDeviceProperties properties = physDevice.getProperties();

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
			false, vk::CompareOp::eAlways, 0.0f, res.texture.mip_levels,
			vk::BorderColor::eIntOpaqueBlack,
			false);

	res.texture.sampler = device.createSampler(sci);
}

void Application::createNormalImage(const std::string& name, engine::scene::prop& res, bool verbose)
{
	int texW, texH, texChannels;
	stbi_uc* pixels = stbi_load(name.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = texW * texH * 4;
	res.normal.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;

	if (!pixels)
	{
		throw std::runtime_error("failed to load image");
	}

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                                  | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = stagingBufferMemory.mapMemory(0, imageSize);
	std::memcpy(data, pixels, imageSize);
	stagingBufferMemory.unmapMemory();

	stbi_image_free(pixels);

	auto&& tmp = createImage(
			texW, texH, res.normal.mip_levels, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
			| vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.normal.image = std::move(tmp.first);
	res.normal.mem = std::move(tmp.second);

	transitionImageLayout(
			*res.normal.image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			res.normal.mip_levels);
	copyBufferToImage(*stagingBuffer, *res.normal.image, texW, texH);
//	transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
	generateMipmaps(*res.normal.image, vk::Format::eR8G8B8A8Unorm, texW, texH, res.normal.mip_levels);
}

void Application::createNormalImageView(engine::scene::prop& res, bool verbose)
{
	vk::ImageViewCreateInfo ivci(
			{},
			*res.normal.image,
			vk::ImageViewType::e2D,
			vk::Format::eR8G8B8A8Unorm,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					res.normal.mip_levels,
					0,
					1));
	res.normal.image_view = { device, ivci };
}

void Application::createNormalSampler(engine::scene::prop& res, bool verbose)
{
	vk::PhysicalDeviceProperties properties = physDevice.getProperties();

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
			false, vk::CompareOp::eAlways, 0.0f, res.normal.mip_levels,
			vk::BorderColor::eIntOpaqueBlack,
			false);

	res.normal.sampler = device.createSampler(sci);
}

void Application::createRoughnessImage(const std::string& name, engine::scene::prop& res, bool verbose)
{
	int texW, texH, texChannels;
	stbi_uc* pixels = stbi_load(name.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = texW * texH * 4;
	res.roughness.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;

	if (!pixels)
	{
		throw std::runtime_error("failed to load image");
	}

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                                  | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = stagingBufferMemory.mapMemory(0, imageSize);
	std::memcpy(data, pixels, imageSize);
	stagingBufferMemory.unmapMemory();

	stbi_image_free(pixels);

	auto&& tmp = createImage(
			texW, texH, res.roughness.mip_levels, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
			| vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.roughness.image = std::move(tmp.first);
	res.roughness.mem = std::move(tmp.second);

	transitionImageLayout(
			*res.roughness.image, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
			res.roughness.mip_levels);
	copyBufferToImage(*stagingBuffer, *res.roughness.image, texW, texH);
//	transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
	generateMipmaps(*res.roughness.image, vk::Format::eR8G8B8A8Unorm, texW, texH, res.roughness.mip_levels);
}

void Application::createRoughnessImageView(engine::scene::prop& res, bool verbose)
{
	vk::ImageViewCreateInfo ivci(
			{},
			*res.roughness.image,
			vk::ImageViewType::e2D,
			vk::Format::eR8G8B8A8Unorm,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor,
					0,
					res.roughness.mip_levels,
					0,
					1));
	res.roughness.image_view = { device, ivci };
}

void Application::createRoughnessSampler(engine::scene::prop& res, bool verbose)
{
	vk::PhysicalDeviceProperties properties = physDevice.getProperties();

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
			false, vk::CompareOp::eAlways, 0.0f, res.roughness.mip_levels,
			vk::BorderColor::eIntOpaqueBlack,
			false);

	res.roughness.sampler = device.createSampler(sci);
}


void Application::loadModel(const std::string& name, engine::scene::prop& res, bool verbose)
{
//	mesh::vertex v;
//	v.position = glm::vec3(-1.0f, -1.0f, 0.0f);
//	v.texcoord = glm::vec3(0.0f, 0.0f, 0.0f);
//	v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
//	mesh.vertices.push_back(v);
//	v.position = glm::vec3(1.0f, -1.0f, 0.0f);
//	v.texcoord = glm::vec3(5.0f, 0.0f, 0.0f);
//	mesh.vertices.push_back(v);
//	v.position = glm::vec3(1.0f, 1.0f, 0.0f);
//	v.texcoord = glm::vec3(5.0f, 5.0f, 0.0f);
//	mesh.vertices.push_back(v);
//	v.position = glm::vec3(-1.0f, 1.0f, 0.0f);
//	v.texcoord = glm::vec3(0.0f, 5.0f, 0.0f);
//	mesh.vertices.push_back(v);
//	mesh.indices = {
//			0, 1, 2,
//			0, 2, 3,
//	};
//	calculate_tangent_space(mesh);
//	return;

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
			size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                             | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.vertices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.geom_buffer.vertex_buffer = std::move(buffers.first);
	res.geom_buffer.vertex_buffer_memory = std::move(buffers.second);

	copyBuffer(stagingBuffer, res.geom_buffer.vertex_buffer, size);
}

void Application::createIndexBuffers(engine::scene::prop& res, bool verbose)
{
	const vk::DeviceSize size = sizeof(res.mesh.indices[0]) * res.mesh.indices.size();

	auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(
			size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible
			                                             | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* data = stagingBufferMemory.mapMemory(0, size);
	std::memcpy(data, res.mesh.indices.data(), size);
	stagingBufferMemory.unmapMemory();

	auto&& buffers = createBuffer(
			size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	res.geom_buffer.index_buffer = std::move(buffers.first);
	res.geom_buffer.index_buffer_memory = std::move(buffers.second);

	copyBuffer(stagingBuffer, res.geom_buffer.index_buffer, size);
}

void Application::createUniformBuffers(bool verbose)
{
	// view_projection
	{
		const vk::DeviceSize size = sizeof(view_projection);

		auto&& buffers = createBuffer(
				size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible
				                                                               | vk::MemoryPropertyFlagBits::eHostCoherent);
		view_projection_buffer = std::move(buffers.first);
		view_projection_buffer_memory = std::move(buffers.second);
	}

	// light_sources
	{
		const vk::DeviceSize alignment = physDevice.getProperties().limits.minUniformBufferOffsetAlignment;
		const vk::DeviceSize padded_size = (sizeof(engine::lights::point_lights_t) / alignment + 1) * alignment;
		const vk::DeviceSize size = padded_size + sizeof(engine::lights::spot_lights_t);

		auto&& buffers = createBuffer(
				size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible
				                                               | vk::MemoryPropertyFlagBits::eHostCoherent);

		light_sources_buffer = std::move(buffers.first);
		light_sources_buffer_memory = std::move(buffers.second);
	}

	// models
	const vk::DeviceSize size = props.size() * sizeof(glm::mat4);

	auto&& buffers = createBuffer(
			size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible
			                                               | vk::MemoryPropertyFlagBits::eHostCoherent);

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
	};

	vk::DescriptorPoolCreateInfo dpci(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxFrames * (1 + props.size()), poolSizes);

	descriptorPool = device.createDescriptorPool(dpci);
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

	ImguiDescriptorPool = device.createDescriptorPool(dpci);
}

void Application::createDescriptorSets(bool verbose)
{
	std::vector<vk::DescriptorSetLayout> counts(maxFrames, *scenewise_descriptor_set_layout);
	vk::DescriptorSetAllocateInfo dsai(*descriptorPool, counts);

	scenewise_descriptor_sets = device.allocateDescriptorSets(dsai);

	counts = std::vector<vk::DescriptorSetLayout>(maxFrames * props.size(), *objectwise_descriptor_set_layout);
	dsai = vk::DescriptorSetAllocateInfo(*descriptorPool, counts);

	objectwise_descriptor_sets = device.allocateDescriptorSets(dsai);

	vk::DescriptorBufferInfo view_projection_info(*view_projection_buffer, 0, sizeof(view_projection));
	vk::DescriptorBufferInfo point_lights_info(*light_sources_buffer, 0, sizeof(engine::lights::point_lights_t));

	const vk::DeviceSize alignment = physDevice.getProperties().limits.minUniformBufferOffsetAlignment;
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

		std::vector<vk::WriteDescriptorSet> descriptorWrites = {
				{*objectwise_descriptor_sets[2 * i + 0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, model_info},
				{*objectwise_descriptor_sets[2 * i + 0], 1, 0, vk::DescriptorType::eCombinedImageSampler, texture_info},
				{*objectwise_descriptor_sets[2 * i + 0], 2, 0, vk::DescriptorType::eCombinedImageSampler, normal_info},
				{*objectwise_descriptor_sets[2 * i + 0], 3, 0, vk::DescriptorType::eCombinedImageSampler, roughness_info},

				{*objectwise_descriptor_sets[2 * i + 1], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, model_info},
				{*objectwise_descriptor_sets[2 * i + 1], 1, 0, vk::DescriptorType::eCombinedImageSampler, texture_info},
				{*objectwise_descriptor_sets[2 * i + 1], 2, 0, vk::DescriptorType::eCombinedImageSampler, normal_info},
				{*objectwise_descriptor_sets[2 * i + 1], 3, 0, vk::DescriptorType::eCombinedImageSampler, roughness_info},
		};

		device.updateDescriptorSets(descriptorWrites, {});
	}
}

void Application::createCommandBuffers(bool verbose)
{
	vk::CommandBufferAllocateInfo cbai(*commandPool, vk::CommandBufferLevel::ePrimary, maxFrames);
	commandBuffers = device.allocateCommandBuffers(cbai);
}

void Application::createSyncObjects(bool verbose)
{
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

	graphicsQueue.submit(si, *isFlightFences[currentFrame]);


	vk::PresentInfoKHR pi(*renderFinishedSemaphores[currentFrame], *swapchain, imgIndex);

	try
	{
		presentQueue.presentKHR(pi);
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

void Application::run()
{
	//window.set_close(true);
	while (!window.should_close())
	{
		glfw::window::poll_events();
		drawFrame();
	}

	device.waitIdle();
}
} // gorilla