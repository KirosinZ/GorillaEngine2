//
// Created by Kiril on 23.02.2023.
//

#ifndef DEEPLOM_LIBRARY_OBJECT_H
#define DEEPLOM_LIBRARY_OBJECT_H

#ifdef GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan_raii.hpp>
#endif
#include <GLFW/glfw3.h>

namespace gorilla::glfw
{

class monitor;

class library_object
{
public:
	inline static int version_major = GLFW_VERSION_MAJOR;
	inline static int version_minor = GLFW_VERSION_MINOR;
	inline static int version_revision = GLFW_VERSION_REVISION;

	inline library_object()
	{
		glfwInit();
	}

	inline ~library_object()
	{
		glfwTerminate();
	}

	std::vector<monitor> monitors() const;
	monitor primary_monitor() const;
//	events::event<library_object, error_fn_t> event_error;
};

}

#endif //DEEPLOM_LIBRARY_OBJECT_H
