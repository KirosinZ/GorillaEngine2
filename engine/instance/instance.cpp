//
// Created by Kiril on 14.03.2023.
//

#include <iostream>
#include <iomanip>
#include "instance.hpp"
#include <engine.hpp>

void display_version(uint32_t version)
{
	std::cout << "Instance Version: ["
			  << VK_VERSION_MAJOR(version) << "."
			  << VK_VERSION_MINOR(version) << "."
			  << VK_VERSION_PATCH(version) << "]\n";
}

void display_layer_properties(const std::vector<vk::LayerProperties>& properties)
{
	const int size = properties.size();
	int cnt = size >= 1000 ? 4 :
			  size >= 100 ? 3 :
			  size >= 10 ? 2 :
			  1;
	++cnt;

	std::cout << "Layer Properties:\n";
	int i = 0;
	for (const auto& prop : properties)
		std::cout << std::setw(cnt) << ++i << ". "
				  << prop.layerName << " ["
				  << VK_VERSION_MAJOR(prop.specVersion) << "."
				  << VK_VERSION_MINOR(prop.specVersion) << "."
				  << VK_VERSION_PATCH(prop.specVersion) << ":"
				  << prop.implementationVersion << "]\n"
				  << std::setw(cnt + 2) << "" << prop.description << "\n";
}

void display_extension_properties(const std::vector<vk::ExtensionProperties>& properties)
{
	const int size = properties.size();
	int cnt = size >= 1000 ? 4 :
	          size >= 100 ? 3 :
	          size >= 10 ? 2 :
	          1;
	++cnt;

	std::cout << "Extension Properties:\n";
	int i = 0;
	for (const auto& prop : properties)
		std::cout << std::setw(cnt) << ++i << ". "
		          << prop.extensionName << " ["
		          << prop.specVersion << "]\n";
}

void display_queue_properties(const std::vector<vk::QueueFamilyProperties>& properties)
{
	const int size = properties.size();
	int cnt = size >= 1000 ? 4 :
	          size >= 100 ? 3 :
	          size >= 10 ? 2 :
	          1;
	++cnt;

	std::cout << "Queue Families:\n";
	int i = 0;
	for (const auto& family: properties)
	{
		std::cout << std::setw(cnt) << ++i << ". ";
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
		          << "]\n";
	}
}

namespace gorilla::engine
{

void instance::establish_context(bool verbose)
{
	uint32_t instanceVersion = _context.enumerateInstanceVersion();
	std::vector<vk::LayerProperties> layerProperties = _context.enumerateInstanceLayerProperties();
	std::vector<vk::ExtensionProperties> extensionProperties = _context.enumerateInstanceExtensionProperties();

	for (const auto& ext : extensionProperties)
	{
		_available_extensions.emplace(ext.extensionName.data());
	}

	if (!verbose)
	{
		return;
	}

	display_version(instanceVersion);
	std::cout << "\n";
	display_layer_properties(layerProperties);
	std::cout << "\n";
	display_extension_properties(extensionProperties);
}

void instance::create_instance(const std::string& app_name, uint32_t app_version, bool verbose)
{
	vk::ApplicationInfo ai(
			app_name.c_str(),
			app_version,
			Engine::Name.c_str(),
			Engine::Version,
			Engine::VulkanVersion
	);

	const std::vector<const char*> layers = {

	};

	std::vector<const char*> extensions = glfw::window::extensions();

	vk::InstanceCreateInfo ici(
			{},
			&ai,
			layers,
			extensions);
	_instance = vk::raii::Instance(_context, ici);
}

vk::raii::PhysicalDevice pick_device(const std::vector<vk::raii::PhysicalDevice>& devices)
{
	if (devices.empty())
	{
		throw std::runtime_error("No GPUs present!");
	}

	return devices[0];
}

void instance::create_physical_device(bool verbose)
{
	_physical_device = pick_device(_instance.enumeratePhysicalDevices());
	std::vector<vk::QueueFamilyProperties> queueFamilies = _physical_device.getQueueFamilyProperties();
	std::vector<vk::LayerProperties> deviceLayerProperties = _physical_device.enumerateDeviceLayerProperties();
	std::vector<vk::ExtensionProperties> deviceExtensionProperties = _physical_device.enumerateDeviceExtensionProperties();

	if (!verbose)
	{
		return;
	}

	display_queue_properties(queueFamilies);
	std::cout << "\n";
	display_layer_properties(deviceLayerProperties);
	std::cout << "\n";
	display_extension_properties(deviceExtensionProperties);
}


instance::instance()
{
	establish_context(true);
	create_instance("Application",
	                VK_MAKE_VERSION(0,1,0),
					true);
	create_physical_device(true);
}

} // gorilla::engine