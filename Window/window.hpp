#ifndef DEEPLOM_WINDOW_HPP
#define DEEPLOM_WINDOW_HPP

#include <string>
#include <vector>

struct GLFWwindow {};

namespace gorilla {

    class Window {
    public:
        Window(std::string name, int w, int h);

        Window(const Window& copy) = delete;
        Window& operator=(const Window& other) = delete;

        Window(Window&& other) = delete;
        Window& operator=(Window&& other) = delete;

        ~Window() noexcept;

        bool shouldClose() const;
        void setShouldClose(bool value);

        vk::raii::SurfaceKHR surface(const vk::raii::Instance& instance) const;

        static void waitEvents();
        static void pollEvents();

        static std::vector<const char *> requiredExtensions();

        std::string Name() const
        {
            return wName;
        }

        int Width() const
        {
            return wWidth;
        }

        int Height() const
        {
            return wHeight;
        }

        vk::Extent2D framebufferSize() const
        {
            int w, h;
            glfwGetFramebufferSize(wHandle, &w, &h);
            return vk::Extent2D(w, h);
        }

    private:
        void init();
        void destroy() noexcept;

        GLFWwindow* wHandle = nullptr;

        std::string wName;
        int wWidth = 0;
        int wHeight = 0;
    };

} // gorilla

#endif //DEEPLOM_WINDOW_HPP
