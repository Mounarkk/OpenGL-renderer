cmake_minimum_required(VERSION 3.10)

# Project Name
project(opengl_renderer)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Add GLFW
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Include Directories
include_directories(
        vendor/glad/include
        vendor/stb_image
)

# Add Source Files
set(SOURCES
        src/main.cpp
        vendor/glad/src/glad.c
        vendor/stb_image/stb_image.cpp
        src/Shader.cpp
        src/Camera.cpp
        src/VertexBuffer.cpp
        src/IndexBuffer.cpp
        src/VertexBufferLayout.h
        src/VertexArray.cpp
        src/Texture.cpp
)

# Create Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Libraries
target_link_libraries(${PROJECT_NAME} glfw GL)
