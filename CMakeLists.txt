cmake_minimum_required(VERSION 3.28)

project(AdroEngine
        VERSION 1.0.0.0
        DESCRIPTION ""
        LANGUAGES "CXX" "C")

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

add_executable(Engine main.cpp
        Include/VkApp.h
        Src/VkApp.cpp
        Include/FileSystem.h
        Src/FileSystem.cpp
        Include/Graphics.h
        Src/Graphics.cpp
)

target_include_directories(Engine PUBLIC Include)
target_include_directories(Engine PUBLIC "$ENV{VULKAN_SDK}/Include")
target_link_libraries(Engine PUBLIC "$ENV{VULKAN_SDK}/Lib/SDL2.lib")

if (WIN32)
    set(VK_USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR")    # Must be defined before including vulkan.h
endif ()
# TODO: cover all other platforms.

# Must define VOLK_IMPLEMENTATION in exact one source file
target_compile_definitions(Engine PRIVATE
        ${VK_USE_PLATFORM}
        VK_NO_PROTOTYPES
        SDL_MAIN_HANDLED
        GFX_VALIDATION_LAYERS)

configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/vert.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/vert.spv" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/frag.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/frag.spv" COPYONLY)