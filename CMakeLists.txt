cmake_minimum_required(VERSION 3.10)

# Project Name
project(opengl_renderer)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Paths for vendor libraries
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/vendor/glfw")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/vendor/glm")
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/vendor/glad")
set(STB_IMAGE_DIR "${CMAKE_SOURCE_DIR}/vendor/stb_image")

# Add GLFW
add_subdirectory(${GLFW_DIR} glfw_build)

# Include directories for libraries
include_directories(
        ${GLFW_DIR}/include
        ${GLM_DIR}
        ${GLAD_DIR}/include
        ${STB_IMAGE_DIR}
)

# Add Source Files
set(SOURCES
        src/main.cpp
        ${GLAD_DIR}/src/glad.c
        ${STB_IMAGE_DIR}/stb_image.cpp
        src/Shader.cpp
        src/Camera.cpp
        src/VertexBuffer.cpp
        src/IndexBuffer.cpp
        src/VertexArray.cpp
        src/Texture.cpp
        src/Cube.cpp
)

# Create Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Libraries
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS}) # For dynamic linking (Linux/Unix)

