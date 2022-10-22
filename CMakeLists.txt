cmake_minimum_required(VERSION 3.23)
project(Deeplom)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

add_executable(Deeplom
        main.cpp
        Geometry/raw_obj.cpp Geometry/raw_obj.h
        Window/window.cpp Window/window.hpp
        Application/application.cpp Application/application.hpp
        Engine/engine.hpp
        Structures/Implementations/array_impl.hpp
        Structures/garray
        Misc/type_traits.hpp
        Structures/common.hpp Misc/line.cpp Misc/line.h)

include_directories(Deeplom
        Engine/..
        Structures/)

target_link_libraries(Deeplom
        Vulkan::Vulkan
        glfw3.lib)
