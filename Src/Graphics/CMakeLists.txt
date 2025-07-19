cmake_minimum_required(VERSION 3.28)

find_package(Vulkan REQUIRED)

add_library(
        Graphics
        OBJECT
        "Graphics.cpp"
        "VkApp.cpp")

target_include_directories(
        Graphics
        PUBLIC
        Include)

target_include_directories(Graphics PUBLIC "$ENV{VULKAN_SDK}/Include")
target_link_libraries(Graphics PUBLIC "$ENV{VULKAN_SDK}/Lib/SDL2.lib")

if (WIN32)
    set(VK_USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR")    # Must be defined before including vulkan.h
endif ()
# TODO: cover all other platforms.

target_compile_definitions(
        Graphics
        PRIVATE
        ${VK_USE_PLATFORM}
        VK_NO_PROTOTYPES
        GFX_VALIDATION_LAYERS)

configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/vert.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/vert.spv" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/frag.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/frag.spv" COPYONLY)