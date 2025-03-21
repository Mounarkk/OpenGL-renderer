# Add the rest of your configuration
cmake_minimum_required(VERSION 3.10)
project(opengl_renderer)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Paths for other vendor libraries
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/vendor/glfw")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/vendor/glm")
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/vendor/glad")
set(STB_IMAGE_DIR "${CMAKE_SOURCE_DIR}/vendor/stb_image")
set(ASSIMP_DIR "${CMAKE_SOURCE_DIR}/vendor/assimp")
set(SPDLOG_DIR "${CMAKE_SOURCE_DIR}/vendor/spdlog")
set(ENTT_DIR "${CMAKE_SOURCE_DIR}/vendor/entt")

# Add GLFW
add_subdirectory(${GLFW_DIR} glfw_build)
add_subdirectory(${ASSIMP_DIR} bin)

# Include directories for libraries
include_directories(
        ${GLFW_DIR}/include
        ${GLAD_DIR}/include
        ${STB_IMAGE_DIR}
        ${GLM_DIR}
        ${ASSIMP_DIR}/include
        ${SPDLOG_DIR}/include
        ${ENTT_DIR}/single_include
)

# Add Source Files
set(SOURCES
        src/main.cpp
        ${GLAD_DIR}/src/glad.c
        ${STB_IMAGE_DIR}/stb_image.cpp
        src/scene/Camera.cpp
        src/Cube.cpp
        src/core/Logger.cpp
        src/gl/Shader.cpp
        src/gl/VertexBuffer.cpp
        src/gl/IndexBuffer.cpp
        src/gl/VertexArray.cpp
        src/gl/Texture.cpp
        src/gl/Debug.cpp
        src/rendering/Mesh.cpp
        src/rendering/Material.cpp
        src/rendering/Renderer.cpp
        src/resource/ModelLoader.cpp
        src/resource/ResourceManager.cpp
        src/scene/Components.h
        src/scene/Entity.cpp
        src/scene/Scene.cpp
        src/systems/RenderingSystem.h
        src/application/Application.cpp
)

# Create Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Libraries
target_link_libraries(${PROJECT_NAME} glfw assimp ${CMAKE_DL_LIBS})


