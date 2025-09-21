#pragma once

#include <exception>
#include <string>

/**
 * Exception class for graphics and rendering-related errors.
 *
 * This exception is thrown when critical rendering operations fail,
 * such as OpenGL initialization, shader compilation, or resource loading.
 * It provides clear error messages to help with debugging.
 */
class RendererException : public std::exception {
public:
  /**
   * Constructs a RendererException with a descriptive error message.
   * @param message Detailed description of what went wrong
   */
  explicit RendererException(const std::string &message)
      : m_Message("Renderer Error: " + message) {}

  /**
   * Returns the error message describing what went wrong.
   * @return C-style string containing the error message
   */
  const char *what() const noexcept override { return m_Message.c_str(); }

private:
  std::string m_Message;
};

/**
 * Exception class specifically for shader-related errors.
 *
 * Thrown when shader compilation, linking, or loading fails.
 * Provides additional context about which shader stage failed.
 */
class ShaderException : public RendererException {
public:
  /**
   * Constructs a ShaderException with shader-specific error information.
   * @param shaderType Type of shader that failed (e.g., "vertex", "fragment")
   * @param message Detailed error message from OpenGL or file system
   */
  ShaderException(const std::string &shaderType, const std::string &message)
      : RendererException("Shader (" + shaderType + ") - " + message) {}
};

/**
 * Exception class for resource loading errors.
 *
 * Thrown when models, textures, or other assets fail to load.
 * Helps identify which resource caused the problem.
 */
class ResourceException : public RendererException {
public:
  /**
   * Constructs a ResourceException with resource-specific error information.
   * @param resourcePath Path to the resource that failed to load
   * @param message Detailed error message explaining the failure
   */
  ResourceException(const std::string &resourcePath, const std::string &message)
      : RendererException("Resource (" + resourcePath + ") - " + message) {}
};