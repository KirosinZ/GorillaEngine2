cmake_minimum_required(VERSION 3.23)
project(Deeplom)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

add_executable(Deeplom main.cpp geometry/facevertexmesh.cpp geometry/facevertexmesh.h Window/window.cpp Window/window.hpp Application/application.cpp Application/application.hpp Engine/engine.hpp)
include_directories(Deeplom Engine/..)

target_link_libraries(Deeplom Vulkan::Vulkan)
target_link_libraries(Deeplom glfw3.lib)
