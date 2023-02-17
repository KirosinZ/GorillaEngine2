#ifndef DEEPLOM_ENGINE_HPP
#define DEEPLOM_ENGINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <3rdparty/stb_image.h>

#include <chrono>

namespace gorilla
{
    struct Engine
    {
        inline static const std::string Name = "Gorilla Engine";
        static const int Version = VK_MAKE_API_VERSION(1, 0, 1, 0);
        static const int VulkanVersion = VK_API_VERSION_1_3;
    };

} // gorilla

#endif //DEEPLOM_ENGINE_HPP
