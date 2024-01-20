#ifndef DEEPLOM_WINDOW_HPP
#define DEEPLOM_WINDOW_HPP

#include <string>
#include <vector>

#include <event/event.h>

#include "enums.h"

namespace gorilla::glfw
{

class window
{
public:
    window(
			std::string name,
			const int w,
			const int h);

    window(const window&) = delete;
    window& operator=(const window&) = delete;

    window(window&&) noexcept;
    window& operator=(window&&) noexcept;

    ~window() noexcept;

    bool should_close() const;
    void set_close(const bool value);

    vk::raii::SurfaceKHR surface(const vk::raii::Instance& instance) const;
	inline GLFWwindow* handle() { return _handle; }

    static void wait_events();
    static void poll_events();

    static std::vector<const char *> extensions();

	static bool present_supported(const vk::raii::Instance& instance, const vk::raii::PhysicalDevice& phys_device, uint32_t queue_family_index);

    inline std::string name() const
    {
        return _name;
    }

    inline int width() const
    {
        return _width;
    }

    inline int height() const
    {
        return _height;
    }

    vk::Extent2D framebuffer_size() const
    {
        int w, h;
        glfwGetFramebufferSize(_handle, &w, &h);
        return vk::Extent2D(w, h);
    }

	void set_input_mode(int value)
	{
		glfwSetInputMode(_handle, GLFW_CURSOR, value);
	}

    glfw::key_action get_key(const glfw::key_id id)
	{
		return (glfw::key_action)glfwGetKey(_handle, (int)id);
	}

    events::event<window, void(const window&, double, double)> cursorpos_event;
private:
	glfw::library_object reference;

    GLFWwindow* _handle = nullptr;

    std::string _name;
    int _width = 0;
    int _height = 0;

};

} // gorilla

#endif //DEEPLOM_WINDOW_HPP
