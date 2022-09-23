//
// Created by Kiril on 15.09.2022.
//

#include "window.hpp"
#include <GLFW/glfw3.h>

#include <utility>
#include <stdexcept>

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

void Window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

void Window::pollEvents()
{
    glfwPollEvents();
}

} // gorilla