#pragma once
#include <glad/glad.h>
#include <glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

/// A stage of a program: its type (GL_VERTEX_SHADER...) and source file.
struct ShaderStage {
  GLenum type;
  std::string path;
};

/**
 * GLSL program built from a vertex, a fragment and an optional geometry
 * stage, or from a single compute stage.
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
  /// Links any combination of stages, e.g. a single GL_COMPUTE_SHADER.
  explicit Shader(const std::vector<ShaderStage> &stages,
                  const std::string &defines = "");
  ~Shader();

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  /// Deletes the GL program. Safe to call several times.
  void clean();

  void use() const;

  /// Runs a compute program on enough work groups to cover `width` x
  /// `height` x `depth` invocations, with the given local size.
  void dispatch(GLuint width, GLuint height, GLuint depth, GLuint localSizeX,
                GLuint localSizeY, GLuint localSizeZ = 1) const;

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setUInt(const std::string &name, unsigned int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, const glm::vec3 &vector) const;
  void setVec4(const std::string &name, const glm::vec4 &vector) const;
  void setMat4(const std::string &name, const glm::mat4 &matrix) const;

  [[nodiscard]] GLuint getID() const { return mID; }

private:
  GLuint mID = 0;
  mutable std::unordered_map<std::string, GLint> mUniformLocationCache;

  GLint getUniformLocation(const std::string &name) const;
};
