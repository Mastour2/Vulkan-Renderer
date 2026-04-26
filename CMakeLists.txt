cmake_minimum_required(VERSION 3.23)

project(
    Renderer
    VERSION 1.0
    DESCRIPTION "Vulkan renderer"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(CGLM REQUIRED IMPORTED_TARGET cglm)

add_executable(App
    src/main.c
    src/wrap/vulkan_instance.c
    src/wrap/vulkan_device.c
    src/wrap/vulkan_swapchain.c
)

target_include_directories(App PRIVATE include)

target_link_libraries(App PRIVATE
    SDL3::SDL3
    PkgConfig::CGLM
    Vulkan::Vulkan
)

if(UNIX AND NOT APPLE)
    target_link_libraries(App PRIVATE m)
endif()
