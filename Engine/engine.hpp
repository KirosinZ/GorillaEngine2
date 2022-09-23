//
// Created by Kiril on 22.09.2022.
//

#ifndef DEEPLOM_ENGINE_HPP
#define DEEPLOM_ENGINE_HPP

#include <vulkan/vulkan_raii.hpp>

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
