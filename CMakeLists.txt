# Set the path to the installed GLM files
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vendor/glm/build/install")

# Find GLM package
find_package(glm REQUIRED)

# Add the rest of your configuration
cmake_minimum_required(VERSION 3.10)
project(opengl_renderer)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Paths for other vendor libraries
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/vendor/glfw")
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/vendor/glad")
set(STB_IMAGE_DIR "${CMAKE_SOURCE_DIR}/vendor/stb_image")

# Add GLFW
add_subdirectory(${GLFW_DIR} glfw_build)

# Include directories for libraries
include_directories(
        ${GLFW_DIR}/include
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
target_link_libraries(${PROJECT_NAME} glfw glm ${CMAKE_DL_LIBS})


