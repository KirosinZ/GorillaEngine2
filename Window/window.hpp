#ifndef DEEPLOM_WINDOW_HPP
#define DEEPLOM_WINDOW_HPP

#include <string>
#include <vector>

struct GLFWwindow {};

namespace gorilla {

    class window {
    public:
        window(
				std::string name,
				const int w,
				const int h);

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&&) = delete;
        window& operator=(window&&) = delete;

        ~window() noexcept;

        bool should_close() const;
        void set_close(const bool value);

        vk::raii::SurfaceKHR surface(const vk::raii::Instance& instance) const;

        static void wait_events();
        static void poll_events();

        static std::vector<const char *> extensions();

        std::string name() const
        {
            return _name;
        }

        int width() const
        {
            return _width;
        }

        int height() const
        {
            return _height;
        }

        vk::Extent2D framebuffer_size() const
        {
            int w, h;
            glfwGetFramebufferSize(_handle, &w, &h);
            return vk::Extent2D(w, h);
        }

    private:
        void init();
        void destroy() noexcept;

        GLFWwindow* _handle = nullptr;

        std::string _name;
        int _width = 0;
        int _height = 0;
    };

} // gorilla

#endif //DEEPLOM_WINDOW_HPP
