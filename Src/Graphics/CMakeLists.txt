cmake_minimum_required(VERSION 3.28)

find_package(Vulkan REQUIRED)

add_library(
        Graphics
        STATIC
        "Graphics.cpp"
        "VkApp.cpp"
        Include/Mesh.h
        Mesh.cpp)

target_include_directories(
        Graphics
        PUBLIC
        Include)

target_include_directories(Graphics PUBLIC "$ENV{VULKAN_SDK}/Include")
target_link_libraries(Graphics PUBLIC "$ENV{VULKAN_SDK}/Lib/SDL2.lib")

target_include_directories(
        Graphics
        PUBLIC
        ${CMAKE_SOURCE_DIR}/ThirdParty/Include)

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

if ("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
    target_link_libraries(
            Graphics
            PUBLIC
            ${CMAKE_SOURCE_DIR}/ThirdParty/Lib/assimp/assimp-vc143-mtd.lib)

    add_custom_command(
            TARGET Graphics POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/ThirdParty/Bin/assimp/assimp-vc143-mtd.dll"
            "${CMAKE_BINARY_DIR}")
else ()
    target_link_libraries(
            Graphics
            PUBLIC
            ${CMAKE_SOURCE_DIR}/ThirdParty/Lib/assimp/assimp-vc143-mt.lib)

    add_custom_command(
            TARGET Graphics POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/ThirdParty/Bin/assimp/assimp-vc143-mt.dll"
            "${CMAKE_BINARY_DIR}")
endif ()

configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/vert.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/vert.spv" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/Resources/Shaders/frag.spv" "${CMAKE_BINARY_DIR}/Resources/Shaders/frag.spv" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/Resources/Meshes/dragon.obj" "${CMAKE_BINARY_DIR}/Resources/Meshes/dragon.obj" COPYONLY)