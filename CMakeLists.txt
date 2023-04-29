cmake_minimum_required(VERSION 3.23)
project(Deeplom)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED shaderc_combined)

add_library(shader_compiler STATIC
        shader_compiler/compiler.cpp shader_compiler/compiler.h
        shader_compiler/compilation_options.h shader_compiler/compilation_options.cpp
        shader_compiler/compilation_result.h
        shader_compiler/enums.h)

target_link_libraries(shader_compiler
        Vulkan::shaderc_combined)

add_library(asset_loader STATIC
        asset_loader/obj/obj.cpp asset_loader/obj/obj.h
        external/stb/stb_image.h
        asset_loader/img/image.cpp asset_loader/img/image.h
        asset_loader/loader.cpp asset_loader/loader.h
        asset_loader/triangulated_mesh/triangulatedmesh.cpp asset_loader/triangulated_mesh/triangulatedmesh.h
        asset_loader/wireframe/wireframe.cpp asset_loader/wireframe/wireframe.h asset_loader/geometry/geometry.cpp asset_loader/geometry/geometry.h engine/vk_helpers/helpers.hpp engine/vk_utils/buffer.cpp engine/vk_utils/buffer.hpp engine/vk_utils/utils.cpp engine/vk_utils/utils.hpp engine/vk_utils/environment.cpp engine/vk_utils/environment.hpp engine/vk_utils/texture.cpp engine/vk_utils/texture.hpp engine/vk_utils/texture.cpp engine/vk_utils/texture.hpp engine/vk_helpers/instance_helpers.cpp engine/vk_helpers/instance_helpers.hpp engine/vk_helpers/environment_helpers.cpp engine/vk_helpers/environment_helpers.hpp engine/vk_helpers/physical_device_helpers.cpp engine/vk_helpers/physical_device_helpers.hpp engine/vk_helpers/device_helpers.cpp engine/vk_helpers/device_helpers.hpp engine/vk_utils/screen.cpp engine/vk_utils/screen.hpp engine/vk_helpers/swapchain_helpers.hpp engine/vk_helpers/swapchain_helpers.cpp engine/vk_helpers/image_helpers.cpp engine/vk_helpers/image_helpers.hpp engine/vk_helpers/buffer_helpers.cpp engine/vk_helpers/buffer_helpers.hpp engine/vk_helpers/command_helpers.cpp engine/vk_helpers/command_helpers.hpp engine/vk_helpers/texture_helpers.cpp engine/vk_helpers/texture_helpers.hpp)

include_directories(asset_loader
        asset_loader)

target_link_libraries(asset_loader
        shader_compiler)

add_library(dear_imgui STATIC
        external/imgui/imconfig.h
        external/imgui/imgui.cpp external/imgui/imgui.h
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_internal.h
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imstb_rectpack.h
        external/imgui/imstb_textedit.h
        external/imgui/imstb_truetype.h
        external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_glfw.h
        external/imgui/imgui_impl_vulkan.cpp external/imgui/imgui_impl_vulkan.h)

target_link_libraries(dear_imgui
        Vulkan::Vulkan
        glfw3.lib)

add_executable(Deeplom
        main.cpp
        engine/window/library_object.h engine/window/enums.h
        engine/window/window.cpp engine/window/window.hpp
        engine/application/application.cpp engine/application/application.hpp
        engine/engine.hpp
        engine/camera/camera.cpp engine/camera/camera.h
        engine/event/event.h
        engine/window/type_aliases.h
        engine/window/monitor.cpp engine/window/monitor.hpp
        engine/instance/instance.cpp engine/instance/instance.hpp
        engine/mesh/renderable_unit.cpp engine/mesh/renderable_unit.hpp
        engine/mesh/meshgroup.cpp engine/mesh/meshgroup.hpp engine/misc/imguiobject.cpp engine/misc/imguiobject.hpp engine/mesh/mesh.hpp engine/mesh/light.hpp engine/application/texture.cpp engine/application/texture.h engine/application/scene.cpp engine/application/scene.h engine/application/geombuffer.cpp engine/application/geombuffer.h)

include_directories(Deeplom
        engine
        external
        .)

target_link_libraries(Deeplom
        Vulkan::Vulkan
        glfw3.lib
        asset_loader
        dear_imgui)
