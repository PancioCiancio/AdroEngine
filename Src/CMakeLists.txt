cmake_minimum_required(VERSION 3.28)

add_subdirectory(Graphics)

add_executable(
        Engine
        main.cpp
        FileSystem.h
        FileSystem.cpp
)

target_link_libraries(
        Engine
        PRIVATE
        Graphics)

target_include_directories(
        Engine
        PRIVATE
        Graphics/Include
)

target_compile_definitions(
        Engine
        PRIVATE
        SDL_MAIN_HANDLED
)

