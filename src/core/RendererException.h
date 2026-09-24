#pragma once

#include <exception>
#include <string>

/// Fatal error of the renderer (context creation, GPU resources...).
class RendererException : public std::exception {
public:
  explicit RendererException(const std::string &message)
      : m_Message("Renderer error: " + message) {}

  const char *what() const noexcept override { return m_Message.c_str(); }

private:
  std::string m_Message;
};

/// Shader file that cannot be read, compiled or linked.
class ShaderException : public RendererException {
public:
  /// @param shader Path or stage of the failing shader
  /// @param message Driver log or reason of the failure
  ShaderException(const std::string &shader, const std::string &message)
      : RendererException("shader " + shader + ": " + message) {}
};

/// Asset (model, texture) that cannot be loaded.
class ResourceException : public RendererException {
public:
  /// @param resourcePath Path of the asset
  /// @param message Reason of the failure
  ResourceException(const std::string &resourcePath, const std::string &message)
      : RendererException("resource " + resourcePath + ": " + message) {}
};
