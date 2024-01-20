cmake_minimum_required(VERSION 3.23)
project(GorillaEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED shaderc_combined)

add_compile_definitions(PROJECT_VERSION_MAJOR=0)
add_compile_definitions(PROJECT_VERSION_MINOR=0)
add_compile_definitions(PROJECT_VERSION_PATCH=1)

add_compile_definitions(PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
add_compile_definitions(RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Resouces")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_compile_definitions(PROJECT_PROFILE_DEBUG)
else()
        add_compile_definitions(PROJECT_PROFILE_RELEASE)
endif()


add_subdirectory(../external/glfw/glfw-3.3.8 external/glfw)

add_library(shader_compiler STATIC
        shader_compiler/compiler.cpp
        shader_compiler/compiler.h
        shader_compiler/compilation_options.h
        shader_compiler/compilation_options.cpp
        shader_compiler/compilation_result.h
        shader_compiler/enums.h)

target_link_libraries(shader_compiler
        Vulkan::shaderc_combined)


add_library(asset_loader STATIC
        asset_loader/obj/obj.cpp
        asset_loader/obj/obj.h
        ../external/stb/stb_image.h
        ../external/stb/stb_image_write.h
        image/image.cpp
        image/image.hpp
        asset_loader/loader.cpp
        asset_loader/loader.h
#        engine/vk_helpers/helpers.hpp
#        engine/vk_utils/buffer.cpp
#        engine/vk_utils/buffer.hpp
#        engine/vk_utils/utils.cpp
#        engine/vk_utils/utils.hpp
#        engine/vk_utils/environment.cpp
#        engine/vk_utils/environment.hpp
#        engine/vk_utils/texture.cpp
#        engine/vk_utils/texture.hpp
#        engine/vk_utils/texture.cpp
#        engine/vk_utils/texture.hpp
#        engine/vk_helpers/instance_helpers.cpp
#        engine/vk_helpers/instance_helpers.hpp
#        engine/vk_helpers/environment_helpers.cpp
#        engine/vk_helpers/environment_helpers.hpp
#        engine/vk_helpers/physical_device_helpers.cpp
#        engine/vk_helpers/physical_device_helpers.hpp
#        engine/vk_helpers/device_helpers.cpp
#        engine/vk_helpers/device_helpers.hpp
#        engine/vk_helpers/swapchain_helpers.hpp
#        engine/vk_helpers/swapchain_helpers.cpp
#        engine/vk_helpers/image_helpers.cpp
#        engine/vk_helpers/image_helpers.hpp
#        engine/vk_helpers/buffer_helpers.cpp
#        engine/vk_helpers/buffer_helpers.hpp
#        engine/vk_helpers/command_helpers.cpp
#        engine/vk_helpers/command_helpers.hpp
#        engine/vk_helpers/texture_helpers.cpp
#        engine/vk_helpers/texture_helpers.hpp
        asset_loader/shader_module/shader_module.cpp
        asset_loader/shader_module/shader_module.h
        asset_loader/obj/detail/obj_detail.cpp
        asset_loader/obj/detail/obj_detail.h)

include_directories(asset_loader
        ../external
        asset_loader)

target_link_libraries(asset_loader
        shader_compiler)

add_library(dear_imgui STATIC
        ../external/imgui/imconfig.h
        ../external/imgui/imgui.cpp
        ../external/imgui/imgui.h
        ../external/imgui/imgui_demo.cpp
        ../external/imgui/imgui_draw.cpp
        ../external/imgui/imgui_internal.h
        ../external/imgui/imgui_tables.cpp
        ../external/imgui/imgui_widgets.cpp
        ../external/imgui/imstb_rectpack.h
        ../external/imgui/imstb_textedit.h
        ../external/imgui/imstb_truetype.h
        ../external/imgui/imgui_impl_glfw.cpp
        ../external/imgui/imgui_impl_glfw.h
        ../external/imgui/imgui_impl_vulkan.cpp
        ../external/imgui/imgui_impl_vulkan.h)

target_link_libraries(dear_imgui
        Vulkan::Vulkan
        glfw)

add_executable(GorillaEngine
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
        engine/misc/imguiobject.cpp engine/misc/imguiobject.hpp
        engine/mesh/mesh.hpp engine/mesh/light.hpp
        engine/application/texture.cpp engine/application/texture.h
        engine/application/scene.cpp engine/application/scene.h
        engine/application/geombuffer.cpp engine/application/geombuffer.h
        utilities/asserts.cpp
        utilities/asserts.hpp
        utilities/platform.hpp
        engine/application/applicationbase.cpp
        engine/application/applicationbase.hpp
        engine/application/memory_heap.hpp engine/application/memory_heap.cpp
        engine/application/memory_type.hpp engine/application/memory_type.cpp
        engine/vk_utils/queue_family.hpp engine/vk_utils/queue_family.cpp
        engine/application/applicationsimple.cpp
        engine/application/applicationsimple.hpp
        engine/vk_utils/queue.cpp
        engine/vk_utils/queue.hpp
        engine/vk_helpers/queue_helpers.cpp
        engine/vk_helpers/queue_helpers.hpp
        Resources/Shaders/PBR_pipeline/pbrpipeline.cpp
        Resources/Shaders/PBR_pipeline/pbrpipeline.hpp)
#        engine/application/androidapplication.cpp engine/application/androidapplication.hpp)

include_directories(GorillaEngine
        engine
        ../external
        ../external/glfw/glfw-3.3.8/include
        .)

target_link_libraries(GorillaEngine
        Vulkan::Vulkan
        asset_loader
        dear_imgui
        )
