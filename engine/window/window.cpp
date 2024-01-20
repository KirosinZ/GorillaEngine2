#include <utility>
#include <stdexcept>

#include <engine.hpp>
#include "window.hpp"


namespace gorilla::glfw {

window::window(
		std::string name,
		const int w,
		const int h)
: _name(std::move(name)),
  _width(w),
  _height(h)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	_handle = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
	if (_handle == nullptr)
	{
		const char* errMsg;
		glfwGetError(&errMsg);
		std::string errMessage(errMsg);

		throw std::runtime_error(errMessage);
	}

	glfwSetWindowUserPointer(_handle, this);

	glfwSetInputMode(_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	const auto callback = [] (GLFWwindow* wnd, double xpos, double ypos) {
		const auto gwnd = reinterpret_cast<window*>(glfwGetWindowUserPointer(wnd));
		gwnd->cursorpos_event(*gwnd, xpos, ypos);
	};

	glfwSetCursorPosCallback(_handle, callback);
}

window::~window() noexcept
{
	glfwDestroyWindow(_handle);
	_handle = nullptr;
}

window::window(window&& move) noexcept
{
	_handle = move._handle;
	move._handle = nullptr;

	_name = std::move(move._name);
	_width = move._width;
	_height = move._height;
}

window& window::operator=(window&& move) noexcept
{
	if (this == &move)
		return *this;

	glfwDestroyWindow(_handle);
	_handle = move._handle;
	move._handle = nullptr;

	_name = std::move(move._name);
	_width = move._width;
	_height = move._height;

	return *this;
}

bool window::should_close() const {
    return glfwWindowShouldClose(_handle);
}

void window::set_close(const bool value) {
    glfwSetWindowShouldClose(_handle, value);
}

vk::raii::SurfaceKHR window::surface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR res;
    if (glfwCreateWindowSurface(*instance, _handle, nullptr, &res) != VK_SUCCESS)
        throw std::runtime_error("Failed to create surface");

    return { instance, res };
}

void window::wait_events()
{
    glfwWaitEvents();
}

void window::poll_events()
{
    glfwPollEvents();
}

std::vector<const char *> window::extensions()
{
    const char** exts;
    uint32_t cnt;
    exts = glfwGetRequiredInstanceExtensions(&cnt);

    std::vector<const char *> res(cnt);
    for (int i = 0; i < cnt; ++i)
    {
        res[i] = exts[i];
    }

    return res;
}

bool window::present_supported(const vk::raii::Instance& instance, const vk::raii::PhysicalDevice& phys_device, uint32_t queue_family_index)
{
	return glfwGetPhysicalDevicePresentationSupport(*instance, *phys_device, queue_family_index);
}

} // gorilla