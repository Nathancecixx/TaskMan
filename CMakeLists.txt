# Set the minimum CMake version and project name
cmake_minimum_required(VERSION 3.5)

# Project name
project(TaskMan VERSION 1.0)

# Set the C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

# Recursively collect all source files in the 'src' folder
file(GLOB_RECURSE TASKMAN_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Create the executable by adding all sources
add_executable(TaskMan ${TASKMAN_SOURCES})

# Link raylib (assuming raylib is installed in your system)
find_package(raylib REQUIRED)
target_link_libraries(TaskMan raylib)

# Link the math library explicitly
target_link_libraries(TaskMan m)

# Add the directory where raygui.h is located
target_include_directories(TaskMan PRIVATE ${CMAKE_SOURCE_DIR}/external/raygui)


