#pragma once

#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <window/window.hpp>

namespace gorilla
{
    class Engine
    {
    public:
        static constexpr std::string Name = "Gorilla Engine";
        static constexpr uint32_t Version = VK_MAKE_VERSION(1, 0, 0);
        static constexpr uint32_t VulkanVersion = VK_API_VERSION_1_3;
    };

} // gorilla
