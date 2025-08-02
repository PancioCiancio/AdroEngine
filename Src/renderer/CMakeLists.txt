cmake_minimum_required(VERSION 3.28)

find_package(
        Vulkan
        REQUIRED
        COMPONENTS
        volk)

add_library(
        renderer
        STATIC
        "common.cpp")

target_include_directories(
        renderer
        PUBLIC
        include)

target_include_directories(
        renderer
        PRIVATE
        Vulkan_INCLUDE_DIR)

target_link_libraries(
        renderer
        PRIVATE
        "$ENV{VULKAN_SDK}/Lib/SDL2.lib")

target_link_libraries(
        renderer
        PRIVATE
        Vulkan_volk_LIBRARY)


if (WIN32)
    set(VK_USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR")    # Must be defined before including vulkan.h
endif ()
# TODO: cover all other platforms.

target_compile_definitions(
        renderer
        PRIVATE
        VK_NO_PROTOTYPES
        ${VK_USE_PLATFORM})