#include <utility>
#include <stdexcept>

#include <Engine/engine.hpp>
#include <Window/window.hpp>


namespace gorilla {

Window::Window(std::string name, int w, int h)
:   wName(std::move(name)),
    wWidth(w),
    wHeight(h)
{
    init();
}

Window::~Window() noexcept
{
    destroy();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(wHandle);
}

void Window::setShouldClose(bool value) {
    glfwSetWindowShouldClose(wHandle, value);
}

vk::raii::SurfaceKHR Window::surface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR res;
    if (glfwCreateWindowSurface(*instance, wHandle, nullptr, &res) != VK_SUCCESS)
        throw std::runtime_error("Failed to create surface");

    return { instance, res };
}

void Window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    wHandle = glfwCreateWindow(wWidth, wHeight, wName.c_str(), nullptr, nullptr);
    if (wHandle == nullptr)
    {
        const char* errMsg;
        glfwGetError(&errMsg);
        std::string errMessage(errMsg);

        throw std::runtime_error(errMessage);
    }
}

void Window::destroy() noexcept
{
    glfwDestroyWindow(wHandle);
    wHandle = nullptr;

    glfwTerminate();
}

void Window::waitEvents()
{
    glfwWaitEvents();
}

void Window::pollEvents()
{
    glfwPollEvents();
}

std::vector<const char *> Window::requiredExtensions()
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