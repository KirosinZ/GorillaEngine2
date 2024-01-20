#pragma once

#include <vk_utils/queue.hpp>


namespace gorilla::vk_helpers
{

std::vector<vk_utils::queue_family> queue_families(const vk::raii::Instance& instance, const vk::raii::PhysicalDevice& phys_dev);

vk_utils::queue get_queue(const vk::raii::Device& dev, const vk_utils::queue_family& type, uint32_t index = 0);

}