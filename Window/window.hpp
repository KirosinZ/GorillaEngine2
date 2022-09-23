#pragma once

#ifndef DEEPLOM_WINDOW_HPP
#define DEEPLOM_WINDOW_HPP

#include <string>

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
        static void pollEvents();

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
