#include <utility>
#include <stdexcept>

#include <Engine/engine.hpp>
#include <Window/window.hpp>


namespace gorilla {

window::window(
		std::string name,
		const int w,
		const int h)
: _name(std::move(name)),
  _width(w),
  _height(h)
{
    init();
}

window::~window() noexcept
{
    destroy();
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

void window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _handle = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
    if (_handle == nullptr)
    {
        const char* errMsg;
        glfwGetError(&errMsg);
        std::string errMessage(errMsg);

        throw std::runtime_error(errMessage);
    }
}

void window::destroy() noexcept
{
    glfwDestroyWindow(_handle);
	_handle = nullptr;

    glfwTerminate();
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

} // gorilla