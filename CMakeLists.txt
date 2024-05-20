cmake_minimum_required(VERSION 3.10)
project(VoxelEngine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add GLAD source
add_library(glad ${CMAKE_SOURCE_DIR}/src/glad.c)

# Find and include GLFW
# On Windows, ensure you have the glfw3.a static library for the linker
find_library(GLFW_LIB NAMES libglfw3dll PATHS ${CMAKE_SOURCE_DIR}/lib NO_DEFAULT_PATH)

# Check if GLFW library is found
if(NOT GLFW_LIB)
    message(FATAL_ERROR "GLFW library not found in ${CMAKE_SOURCE_DIR}/lib")
else()
    message(STATUS "Found GLFW library: ${GLFW_LIB}")
endif()

# Add the executable
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(VoxelEngine ${SOURCES})

# Link libraries
target_link_libraries(VoxelEngine glad ${GLFW_LIB} ${CMAKE_DL_LIBS})

# Specify the location of the GLFW DLL for running the executable in the IDE
add_custom_command(TARGET VoxelEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/bin/glfw3.dll"
        $<TARGET_FILE_DIR:VoxelEngine>)
