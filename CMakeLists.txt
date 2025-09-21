# ============================================================================
# OpenGL Renderer CMake Configuration
# ============================================================================
# This CMake file builds an educational OpenGL renderer with proper organization
# and clear documentation for learning graphics programming concepts.

cmake_minimum_required(VERSION 3.10)
project(opengl_renderer VERSION 1.0.0 LANGUAGES CXX C)

# ============================================================================
# Project Configuration
# ============================================================================

# Use modern C++17 features for better code quality and learning
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable debug information in all builds for better debugging experience
set(CMAKE_DEBUG_POSTFIX "_d")

# ============================================================================
# Build Configuration (Debug/Release)
# ============================================================================

# Set default build type to Debug for development
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Debug build configuration - optimized for development 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build - optimized for development")
    add_compile_definitions(DEBUG_BUILD)
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

# Release build configuration - optimized for performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build - optimized for performance")
    add_compile_definitions(RELEASE_BUILD NDEBUG)
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ============================================================================
# Third-Party Dependencies
# ============================================================================
# All external libraries used by the renderer with clear documentation

# GLFW - Cross-platform window and input handling
# Provides window creation, OpenGL context management, and input handling
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/vendor/glfw")
message(STATUS "Using GLFW from: ${GLFW_DIR}")

# GLM - OpenGL Mathematics library
# Header-only library for vector and matrix math operations
set(GLM_DIR "${CMAKE_SOURCE_DIR}/vendor/glm")
message(STATUS "Using GLM from: ${GLM_DIR}")

# GLAD - OpenGL function loader
# Generates OpenGL function pointers for the target OpenGL version
set(GLAD_DIR "${CMAKE_SOURCE_DIR}/vendor/glad")
message(STATUS "Using GLAD from: ${GLAD_DIR}")

# STB Image - Image loading library
# Single-header library for loading various image formats (PNG, JPG, etc.)
set(STB_IMAGE_DIR "${CMAKE_SOURCE_DIR}/vendor/stb_image")
message(STATUS "Using STB Image from: ${STB_IMAGE_DIR}")

# Assimp - 3D model loading library
# Comprehensive library for loading various 3D model formats (OBJ, FBX, etc.)
set(ASSIMP_DIR "${CMAKE_SOURCE_DIR}/vendor/assimp")
message(STATUS "Using Assimp from: ${ASSIMP_DIR}")

# spdlog - Fast C++ logging library
# High-performance logging with various output formats and levels
set(SPDLOG_DIR "${CMAKE_SOURCE_DIR}/vendor/spdlog")
message(STATUS "Using spdlog from: ${SPDLOG_DIR}")

# EnTT - Entity Component System library
# Header-only ECS library for game object management
set(ENTT_DIR "${CMAKE_SOURCE_DIR}/vendor/entt")
message(STATUS "Using EnTT from: ${ENTT_DIR}")

# ============================================================================
# Build Third-Party Libraries
# ============================================================================

# Build GLFW as a subdirectory (creates static library)
add_subdirectory(${GLFW_DIR} glfw_build)

# Build Assimp as a subdirectory (creates static library)
# Note: Assimp has many build options - using defaults for simplicity
add_subdirectory(${ASSIMP_DIR} bin)

# ============================================================================
# Include Directories
# ============================================================================
# Add header search paths for all dependencies

include_directories(
    # Third-party library headers
    ${GLFW_DIR}/include          # GLFW window and input management
    ${GLAD_DIR}/include          # OpenGL function loader
    ${STB_IMAGE_DIR}             # Image loading utilities
    ${GLM_DIR}                   # Math library (header-only)
    ${ASSIMP_DIR}/include        # 3D model loading
    ${SPDLOG_DIR}/include        # Logging library (header-only)
    ${ENTT_DIR}/single_include   # Entity Component System (header-only)
    
    # Project source directories
    src/                         # Main source directory
)

# ============================================================================
# Source Files Organization
# ============================================================================
# Organized by functional categories for better maintainability

# Main application entry point
set(MAIN_SOURCES
    src/main.cpp
)

# Core utilities and foundational classes
set(CORE_SOURCES
    src/core/Logger.cpp
    src/core/Logger.h
    src/core/Config.cpp
    src/core/Config.h
)

# OpenGL wrapper classes for safe resource management
set(OPENGL_SOURCES
    src/gl/Debug.cpp
    src/gl/Debug.h
    src/gl/FrameBuffer.cpp
    src/gl/FrameBuffer.h
    src/gl/GLObjectDestroyer.cpp
    src/gl/GLObjectDestroyer.h
    src/gl/IndexBuffer.cpp
    src/gl/IndexBuffer.h
    src/gl/Shader.cpp
    src/gl/Shader.h
    src/gl/Texture.cpp
    src/gl/Texture.h
    src/gl/VertexArray.cpp
    src/gl/VertexArray.h
    src/gl/VertexBuffer.cpp
    src/gl/VertexBuffer.h
    src/gl/VertexBufferLayout.h
)

# Rendering pipeline and graphics management
set(RENDERING_SOURCES
    src/rendering/LightManager.cpp
    src/rendering/LightManager.h
    src/rendering/Material.cpp
    src/rendering/Material.h
    src/rendering/Mesh.cpp
    src/rendering/Mesh.h
    src/rendering/RenderPass.cpp
    src/rendering/RenderPass.h
    src/rendering/Renderer.cpp
    src/rendering/Renderer.h
    src/rendering/Skybox.cpp
    src/rendering/Skybox.h
)

# Resource loading and management (models, textures, shaders)
set(RESOURCE_SOURCES
    src/resource/ModelLoader.cpp
    src/resource/ModelLoader.h
    src/resource/ResourceManager.cpp
    src/resource/ResourceManager.h
)

# Scene management and Entity Component System
set(SCENE_SOURCES
    src/scene/Camera.cpp
    src/scene/Camera.h
    src/scene/Components.h
    src/scene/Entity.cpp
    src/scene/Entity.h
    src/scene/Scene.cpp
    src/scene/Scene.h
)

# Game systems that operate on components
set(SYSTEMS_SOURCES
    # src/systems/RenderingSystem.h - removed in favor of new rendering architecture
)

# Application framework and main loop management
set(APPLICATION_SOURCES
    src/application/Application.cpp
    src/application/Application.h
)

# Input handling and user interaction
set(INPUT_SOURCES
    src/input/InputHandler.cpp
    src/input/InputHandler.h
)

# Legacy model classes (to be refactored in future tasks)
# Temporarily excluded due to API compatibility issues
set(LEGACY_SOURCES
    # src/Cube.cpp
    # src/Cube.h
    # src/Model.cpp
    # src/Model.h
)

# Third-party source files that need to be compiled
set(VENDOR_SOURCES
    ${GLAD_DIR}/src/glad.c       # OpenGL function loader implementation
    ${STB_IMAGE_DIR}/stb_image.cpp  # Image loading implementation
)

# Combine all source files for the executable
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${OPENGL_SOURCES}
    ${RENDERING_SOURCES}
    ${RESOURCE_SOURCES}
    ${SCENE_SOURCES}
    ${SYSTEMS_SOURCES}
    ${APPLICATION_SOURCES}
    ${INPUT_SOURCES}
    ${LEGACY_SOURCES}
    ${VENDOR_SOURCES}
)

# ============================================================================
# Executable Creation
# ============================================================================

# Create the main executable with all organized source files
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Set target properties for better organization
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "opengl_renderer"
    DEBUG_POSTFIX "_d"
)

# ============================================================================
# Library Linking
# ============================================================================
# Link all required libraries to the executable

target_link_libraries(${PROJECT_NAME}
    # GLFW for window management and input handling
    glfw
    
    # Assimp for 3D model loading
    assimp
    
    # System libraries
    ${CMAKE_DL_LIBS}  # Dynamic linking library (required on some systems)
)

# Platform-specific linking
if(WIN32)
    # Windows-specific OpenGL libraries
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(APPLE)
    # macOS-specific frameworks
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} 
        ${OPENGL_LIBRARY} 
        ${COCOA_LIBRARY} 
        ${IOKIT_LIBRARY} 
        ${COREVIDEO_LIBRARY}
    )
else()
    # Linux-specific libraries
    # Set OpenGL policy to prefer GLVND (modern OpenGL loader) over legacy
    if(POLICY CMP0072)
        cmake_policy(SET CMP0072 NEW)
    endif()
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
endif()

# ============================================================================
# Build Information
# ============================================================================

# Display useful build information
message(STATUS "")
message(STATUS "=== OpenGL Renderer Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  - GLFW: Window and input management")
message(STATUS "  - GLAD: OpenGL function loading")
message(STATUS "  - GLM: Mathematics library")
message(STATUS "  - STB Image: Image loading")
message(STATUS "  - Assimp: 3D model loading")
message(STATUS "  - spdlog: Logging")
message(STATUS "  - EnTT: Entity Component System")
message(STATUS "")
message(STATUS "Source Organization:")
message(STATUS "  - Core: ${CORE_SOURCES}")
message(STATUS "  - OpenGL: ${OPENGL_SOURCES}")
message(STATUS "  - Rendering: ${RENDERING_SOURCES}")
message(STATUS "  - Resources: ${RESOURCE_SOURCES}")
message(STATUS "  - Scene: ${SCENE_SOURCES}")
message(STATUS "  - Systems: ${SYSTEMS_SOURCES}")
message(STATUS "  - Application: ${APPLICATION_SOURCES}")
message(STATUS "  - Input: ${INPUT_SOURCES}")
message(STATUS "===========================================")
message(STATUS "")


