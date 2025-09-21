#pragma once
#include "../core/Logger.h"
#include <../../vendor/glm/fwd.hpp>
#include <../../vendor/glm/gtc/type_ptr.hpp>
#include <fstream>
#include <fwd.hpp>
#include <glad/glad.h>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

/**
 * OpenGL shader program wrapper that handles loading, compilation, and uniform management.
 * 
 * The Shader class encapsulates an OpenGL shader program consisting of vertex and fragment
 * shaders. It handles the complete shader lifecycle: loading source code from files,
 * compiling individual shaders, linking the program, and providing a convenient interface
 * for setting uniform variables. The class also includes uniform location caching for
 * performance optimization.
 * 
 * Key features:
 * - Automatic shader compilation with error reporting
 * - Uniform location caching to avoid repeated OpenGL queries
 * - Type-safe uniform setters for common data types
 * - Proper resource cleanup and error handling
 */
class Shader {
public:
  /**
   * Constructs a shader program from vertex and fragment shader files.
   * Loads, compiles, and links the shaders. Throws an exception if compilation fails.
   * @param vertexPath Path to the vertex shader source file
   * @param fragmentPath Path to the fragment shader source file
   */
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  
  /**
   * Destructor that automatically cleans up OpenGL shader resources.
   */
  ~Shader();
  
  /**
   * Manually cleans up OpenGL shader program resources.
   * Called automatically by destructor, but can be called explicitly if needed.
   */
  void clean();

  /**
   * Activates this shader program for rendering.
   * Must be called before setting uniforms or drawing with this shader.
   */
  void use() const;

  // Uniform setter methods - these set shader uniform variables
  
  /**
   * Sets a boolean uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param value Boolean value to set
   */
  void setBool(const std::string &name, bool value) const;
  
  /**
   * Sets an integer uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param value Integer value to set
   */
  void setInt(const std::string &name, int value) const;
  
  /**
   * Sets a float uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param value Float value to set
   */
  void setFloat(const std::string &name, float value) const;
  
  /**
   * Sets a 3-component vector uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param vector 3D vector value to set
   */
  void setVec3(const std::string &name, glm::vec3 vector) const;
  
  /**
   * Sets a 4x4 matrix uniform variable.
   * @param name Name of the uniform variable in the shader
   * @param matrix 4x4 matrix value to set
   */
  void setMat4(const std::string &name, const glm::mat4 &matrix) const;

private:
  GLuint mID;  // OpenGL shader program ID
  std::unordered_map<std::string, GLint> mUniformLocationCache;  // Cache for uniform locations

  /**
   * Gets the location of a uniform variable, using cache for performance.
   * @param name Name of the uniform variable
   * @return OpenGL uniform location, or -1 if not found
   */
  GLint getUniformLocation(const std::string &name);
};
