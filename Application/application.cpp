//
// Created by Kiril on 19.09.2022.
//

#include <iostream>
#include "application.hpp"

namespace gorilla {

    Application::Application(std::string name, int w, int h)
    : window(std::move(name), w, h)
    {
        initVulkan();
    }

    void Application::establishContext(bool verbose) {
        if (!verbose)
            return;
        auto layers = context.enumerateInstanceLayerProperties();

        std::cout << "===Layers===" << std::endl;
        for (auto layer : layers)
        {
            std::cout << "Layer: " << layer.layerName << "." << std::endl
                      << "Description: " << layer.description << "." << std::endl
                      << "Version: " << layer.implementationVersion << ". Spec Version: " << layer.specVersion << "." << std::endl;
        }

        std::cout << "===Extensions===" << std::endl;
        auto extensions = context.enumerateInstanceExtensionProperties();
        for (auto ext : extensions)
        {
            std::cout << "Extension: " << ext.extensionName << "." << std::endl
                      << "Version: " << ext.specVersion << "." << std::endl;
        }
    }

    void Application::createInstance(bool verbose)
    {
        vk::ApplicationInfo ai(
                window.Name().c_str(),
                VK_MAKE_API_VERSION(1, 0, 1, 0),
                Engine::Name.c_str(),
                Engine::Version,
                Engine::VulkanVersion
        );

        const std::vector<const char*> layers{
                //"VK_LAYER_KHRONOS_validation",
        };

        vk::InstanceCreateInfo ici({}, &ai, layers);
        instance = vk::raii::Instance(context, ici);
    }

    void Application::pickPhysicalDevice(bool verbose)
    {
        physDevices = instance.enumeratePhysicalDevices();
        if (physDevices.empty())
            throw std::runtime_error("No GPUs present");

        const auto queues = physDevices[0].getQueueFamilyProperties();
        if (verbose)
            for (auto queue : queues)
            {
                std::cout << queue.queueCount << " queues capable of " << to_string(queue.queueFlags) << std::endl;
            }

    }

    void Application::createLogicalDevice(bool verbose)
    {
        const std::array<float, 1>  priorities = { 1.0f };

        vk::DeviceQueueCreateInfo dqci({}, 0, priorities);

        vk::DeviceCreateInfo dci({}, dqci);

        device = vk::raii::Device(physDevices[0], dci);
    }

    void Application::initVulkan() {
        establishContext();
        createInstance();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void Application::run() {
        window.setShouldClose(true);

        while (!window.shouldClose()) {
            Window::pollEvents();
        }
    }
} // gorilla