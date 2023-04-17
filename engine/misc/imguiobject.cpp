//
// Created by Kiril on 21.03.2023.
//

#include "imguiobject.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>


namespace gorilla::engine::misc
{

imgui_object::imgui_object(GLFWwindow* window, bool install_callbacks, ImGui_ImplVulkan_InitInfo* info, VkRenderPass renderpass)
: _initialized(true)
{
	ImGui::CreateContext();
//	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForVulkan(window, install_callbacks);
	ImGui_ImplVulkan_Init(info, renderpass);
}

imgui_object::~imgui_object() noexcept
{
	if (_initialized)
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}

} // gorilla::engine::misc