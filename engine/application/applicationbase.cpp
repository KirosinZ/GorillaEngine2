#include "applicationbase.hpp"

#include <iostream>
#include <utilities\platform.hpp>


namespace gorilla
{

application_base::application_base() :
	_window("Gorilla Application", 1920, 1080)
{
	vk::ApplicationInfo ai(
		"Gorilla Application",
		VK_MAKE_VERSION(1, 0, 0),
		"Gorilla Engine",
		VK_MAKE_VERSION(build::version_major, build::version_minor, build::version_patch),
		VK_API_VERSION_1_3);

	std::vector instance_layers{
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector instance_extensions = glfw::window::extensions();

	vk::InstanceCreateInfo ici(
		{},
		&ai,
		instance_layers,
		instance_extensions);
	_instance = vk::raii::Context{}.createInstance(ici);

	_phys_device = std::move(_instance.enumeratePhysicalDevices()[0]);

	std::vector family_properties = _phys_device.getQueueFamilyProperties();
	_queue_families.resize(family_properties.size());
	uint32_t index = 0;
	std::ranges::transform(family_properties, std::begin(_queue_families), [&] (auto props) -> vk_utils::queue_family {
		const bool present_supported = glfw::window::present_supported(_instance, _phys_device, index);
		return vk_utils::queue_family(index++, props, present_supported);
	});

	index = 0;
	vk::PhysicalDeviceMemoryProperties mem_props = _phys_device.getMemoryProperties();
	std::ranges::transform(mem_props.memoryTypes, std::back_inserter(_memory_types), [&] (auto mt) -> memory_type {
		const memory_heap heap(mt.heapIndex, mem_props.memoryHeaps[mt.heapIndex]);
		return memory_type(index++, heap, mt);
	});

	for (auto f: _queue_families)
	{
		if (_graphics_family.index() == std::numeric_limits<uint32_t>::max() && f.has_graphics())
		{
			_graphics_family = f;
			continue;
		}

		if (_present_family.index() == std::numeric_limits<uint32_t>::max() && f.has_present())
		{
			_present_family = f;
			continue;
		}

		if (f.has_transfer())
			_transfer_family = f;
	}

	std::vector priorities{ 1.0f };

	std::vector dqcis{
		vk::DeviceQueueCreateInfo({}, _graphics_family.index(), priorities),
		vk::DeviceQueueCreateInfo({}, _present_family.index(), priorities),
		vk::DeviceQueueCreateInfo({}, _transfer_family.index(), priorities),
	};

	std::vector device_instances{
		"VK_KHR_swapchain"
	};

	vk::PhysicalDeviceFeatures enabled_features = _phys_device.getFeatures();

	vk::DeviceCreateInfo dci(
		{},
		dqcis,
		{},
		device_instances,
		&enabled_features);
	_device = _phys_device.createDevice(dci);

	_graphics_queue = _device.getQueue(_graphics_family.index(), 0);
	_present_queue = _device.getQueue(_present_family.index(), 0);
	_transfer_queue = _device.getQueue(_transfer_family.index(), 0);

	_surface = _window.surface(_instance);

	_surface_formats = _phys_device.getSurfaceFormatsKHR(*_surface);
	_surface_capabilities = _phys_device.getSurfaceCapabilitiesKHR(*_surface);

	for (auto f: _surface_formats)
	{
		if (f.format == vk::Format::eB8G8R8A8Srgb)
		{
			_sc_format = f.format;
			_sc_cspace = f.colorSpace;
		}
	}

	_image_count = std::clamp(2u, _surface_capabilities.minImageCount, _surface_capabilities.maxImageCount);

	_sc_extent = _surface_capabilities.currentExtent;

	std::vector queue_families{
		_graphics_family.index()
	};
	vk::SwapchainCreateInfoKHR scci(
		{},
		*_surface,
		_image_count,
		_sc_format,
		_sc_cspace,
		_sc_extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		queue_families,
		_surface_capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::PresentModeKHR::eFifo,
		false);
	_swapchain = _device.createSwapchainKHR(scci);

	std::vector sc_images_raw = _swapchain.getImages();
	std::ranges::transform(sc_images_raw, std::back_inserter(_sc_images), [] (VkImage img) -> vk::Image {
		return { img };
	});

	vk::CommandPoolCreateInfo cpci(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		_graphics_family.index());
	_command_pool = _device.createCommandPool(cpci);

	vk::CommandBufferAllocateInfo cbai(*_command_pool, vk::CommandBufferLevel::ePrimary, _image_count);
	_frame_cmds = _device.allocateCommandBuffers(cbai);

	vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);
	std::ranges::generate_n(std::back_inserter(_frame_in_flight_fences), _image_count, [&] () -> vk::raii::Fence {
		return _device.createFence(fci);
	});

	vk::SemaphoreCreateInfo sci{};
	std::ranges::generate_n(std::back_inserter(_image_available_semaphores), _image_count, [&] () -> vk::raii::Semaphore {
		return _device.createSemaphore(sci);
	});
	std::ranges::generate_n(std::back_inserter(_render_finished_semaphores), _image_count, [&] () -> vk::raii::Semaphore {
		return _device.createSemaphore(sci);
	});
}


void application_base::run()
{
	while (!_window.should_close())
	{

		process_input();

		pre_render();

		vk::raii::CommandBuffer& cmd = _frame_cmds[_current_frame];
		vk::raii::Fence& fence = _frame_in_flight_fences[_current_frame];

		vk::raii::Semaphore& img_available_semaphore = _image_available_semaphores[_current_frame];
		vk::raii::Semaphore& render_semaphore = _render_finished_semaphores[_current_frame];

		vk::resultCheck(_device.waitForFences(*fence, true, UINT64_MAX), "Fence check");
		const auto[result, image_index] = _swapchain.acquireNextImage(UINT64_MAX, *img_available_semaphore);
		vk::resultCheck(result, "Acquire check");

		_device.resetFences(*fence);

		cmd.reset();

		vk::CommandBufferBeginInfo cbbi{};
		cmd.begin(cbbi);
		render(cmd, image_index);
		cmd.end();

		std::vector<vk::PipelineStageFlags> wait_stages{
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};
		vk::SubmitInfo si(*img_available_semaphore, wait_stages, *cmd, *render_semaphore);
		_graphics_queue.submit(si, *fence);

		vk::PresentInfoKHR pi(*render_semaphore, *_swapchain, image_index);

		try
		{
			_present_queue.presentKHR(pi);
			_current_frame = (_current_frame + 1) % _image_count;
		}
		catch (const std::exception& e)
		{

			_current_frame = (_current_frame + 1) % _image_count;
		}
	}

	_device.waitIdle();
}



}
