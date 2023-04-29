//
// Created by Kiril on 27.04.2023.
//

#ifndef DEEPLOM_SCREEN_HPP
#define DEEPLOM_SCREEN_HPP

#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include <window/window.hpp>

namespace vk_utils
{

class screen
{
public:

	const gorilla::glfw::window& window() const { return m_window; }
	const vk::raii::SwapchainKHR& swapchain() const { return m_swapchain; }
	const std::vector<vk::Image>& images() const { return m_swapchain_images; }
	const std::vector<vk::ImageView> image_views() const { return m_swapchain_image_views; }

	vk::Format format() const { return m_swapchain_format; }
	vk::Extent2D extent() const { return m_swapchain_image_extent; }
private:
	gorilla::glfw::window m_window;
	vk::raii::SwapchainKHR m_swapchain = nullptr;
	std::vector<vk::Image> m_swapchain_images;
	std::vector<vk::ImageView> m_swapchain_image_views;

	vk::Format m_swapchain_format;
	vk::Extent2D m_swapchain_image_extent;
};

} // vk_utils

#endif //DEEPLOM_SCREEN_HPP
