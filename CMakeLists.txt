cmake_minimum_required(VERSION 3.10)
project(opengl_renderer VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug, Release or RelWithDebInfo" FORCE)
endif()

# Prefer GLVND over the legacy libGL on Linux
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

# ----------------------------------------------------------------------------
# Dependencies, all vendored
# ----------------------------------------------------------------------------
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/glfw glfw_build)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/assimp bin)

find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${VENDOR_DIR}/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui SYSTEM PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# ----------------------------------------------------------------------------
# Renderer
# ----------------------------------------------------------------------------
set(SOURCES
    src/main.cpp

    src/application/Application.cpp

    src/core/Config.cpp
    src/core/Logger.cpp

    src/gl/Debug.cpp
    src/gl/FrameBuffer.cpp
    src/gl/IndexBuffer.cpp
    src/gl/Shader.cpp
    src/gl/Texture.cpp
    src/gl/UniformBuffer.cpp
    src/gl/VertexArray.cpp
    src/gl/VertexBuffer.cpp

    src/input/InputHandler.cpp

    src/rendering/Frustum.cpp
    src/rendering/Material.cpp
    src/rendering/Mesh.cpp
    src/rendering/Primitives.cpp
    src/rendering/RenderPass.cpp
    src/rendering/ShaderInterface.cpp
    src/rendering/Renderer.cpp
    src/rendering/Skybox.cpp

    src/resource/ModelLoader.cpp
    src/resource/ResourceManager.cpp

    src/scene/Camera.cpp
    src/scene/Entity.cpp
    src/scene/Scene.cpp

    src/ui/DebugUI.cpp

    ${VENDOR_DIR}/glad/src/glad.c
    ${VENDOR_DIR}/stb_image/stb_image.cpp
)

add_executable(opengl_renderer ${SOURCES})

target_include_directories(opengl_renderer PRIVATE src)
target_include_directories(opengl_renderer SYSTEM PRIVATE
    ${VENDOR_DIR}/glad/include
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/stb_image
    ${VENDOR_DIR}/spdlog/include
    ${VENDOR_DIR}/entt/single_include
)

target_link_libraries(opengl_renderer PRIVATE glfw assimp imgui OpenGL::GL ${CMAKE_DL_LIBS})

# Lets the executable find config.txt and res/ wherever it is launched from
target_compile_definitions(opengl_renderer PRIVATE
    RENDERER_ROOT_DIR="${CMAKE_SOURCE_DIR}"
    GLFW_INCLUDE_NONE # glad provides the GL headers
)

if(MSVC)
    target_compile_options(opengl_renderer PRIVATE /W4)
else()
    target_compile_options(opengl_renderer PRIVATE -Wall -Wextra)
    set_source_files_properties(
        ${VENDOR_DIR}/glad/src/glad.c ${VENDOR_DIR}/stb_image/stb_image.cpp
        PROPERTIES COMPILE_OPTIONS -w)
endif()
