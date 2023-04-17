//
// Created by Kiril on 21.03.2023.
//

#ifndef DEEPLOM_IMGUIOBJECT_HPP
#define DEEPLOM_IMGUIOBJECT_HPP

#include <cstddef>

#include <imgui/imgui_impl_vulkan.h>

struct GLFWwindow;

namespace gorilla::engine::misc
{

class imgui_object
{
public:
	imgui_object(const imgui_object&) = delete;
	imgui_object(imgui_object&&) = delete;

	imgui_object& operator=(const imgui_object&) = delete;
	imgui_object& operator=(imgui_object&&) = delete;

	imgui_object(GLFWwindow* window, bool install_callbacks, ImGui_ImplVulkan_InitInfo* info, VkRenderPass renderpass);
	~imgui_object() noexcept;

private:
	bool _initialized = false;
};

} // gorilla::engine::misc

#endif //DEEPLOM_IMGUIOBJECT_HPP
