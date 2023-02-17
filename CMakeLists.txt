cmake_minimum_required(VERSION 3.23)
project(Deeplom)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED shaderc_combined)

add_library(shaders STATIC
                Shaders/compiler.cpp Shaders/compiler.h
                Shaders/compilation_options.h Shaders/compilation_options.cpp
                Shaders/compilation_result.tpp Shaders/compilation_result.h
                Shaders/enums.h)

target_link_libraries(shaders
        Vulkan::shaderc_combined)

add_executable(Deeplom
        main.cpp
        Geometry/raw_obj.h
        Window/window.cpp Window/window.hpp
        Application/application.cpp Application/application.hpp
        Engine/engine.hpp
        Camera/camera.cpp Camera/camera.h)

include_directories(Deeplom
        Engine/..)


target_link_libraries(Deeplom
        Vulkan::Vulkan
        glfw3.lib
        shaders)
