#pragma once
#include <glad/glad.h>
#include <glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

/**
 * GLSL program built from a vertex, a fragment and an optional geometry stage.
 *
 * Sources go through a small preprocessor before compilation:
 * - `#include "file"` is replaced by the file content, resolved relative to
 *   the including file. Each file is included once per stage.
 * - `defines` is inserted right after the `#version` line.
 *
 * Compilation or link errors throw a ShaderException carrying the driver log.
 * Uniform locations are cached after the first lookup. Setters use
 * glProgramUniform, so the program does not need to be bound.
 */
class Shader {
public:
  Shader(const std::string &vertexPath, const std::string &fragmentPath,
         const std::string &geometryPath = "", const std::string &defines = "");
  ~Shader();

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  /// Deletes the GL program. Safe to call several times.
  void clean();

  void use() const;

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setUInt(const std::string &name, unsigned int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, const glm::vec3 &vector) const;
  void setMat4(const std::string &name, const glm::mat4 &matrix) const;

  [[nodiscard]] GLuint getID() const { return mID; }

private:
  GLuint mID = 0;
  mutable std::unordered_map<std::string, GLint> mUniformLocationCache;

  GLint getUniformLocation(const std::string &name) const;
};
