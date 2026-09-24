#include "Shader.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"

#include <GLFW/glfw3.h>
#include <gtc/type_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace {
std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw ShaderException("file loading", "Cannot open " + path);

  std::stringstream stream;
  stream << file.rdbuf();
  return stream.str();
}

GLuint compileStage(const GLenum stage, const std::string &source,
                    const std::string &path) {
  const GLuint shader = glCreateShader(stage);
  const char *code = source.c_str();
  glShaderSource(shader, 1, &code, nullptr);
  glCompileShader(shader);

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<size_t>(std::max(logLength, 1)), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw ShaderException(path, "Compilation failed:\n" + log);
  }
  return shader;
}
} // namespace

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath,
               const std::string &geometryPath) {
  std::vector<GLuint> stages;
  auto deleteStages = [&stages]() {
    for (const GLuint stage : stages)
      glDeleteShader(stage);
  };

  try {
    stages.push_back(
        compileStage(GL_VERTEX_SHADER, readFile(vertexPath), vertexPath));
    stages.push_back(
        compileStage(GL_FRAGMENT_SHADER, readFile(fragmentPath), fragmentPath));
    if (!geometryPath.empty())
      stages.push_back(compileStage(GL_GEOMETRY_SHADER, readFile(geometryPath),
                                    geometryPath));
  } catch (...) {
    deleteStages();
    throw;
  }

  mID = glCreateProgram();
  for (const GLuint stage : stages)
    glAttachShader(mID, stage);
  glLinkProgram(mID);
  deleteStages();

  GLint success = 0;
  glGetProgramiv(mID, GL_LINK_STATUS, &success);
  if (!success) {
    GLint logLength = 0;
    glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<size_t>(std::max(logLength, 1)), '\0');
    glGetProgramInfoLog(mID, logLength, nullptr, log.data());
    glDeleteProgram(mID);
    mID = 0;
    throw ShaderException(vertexPath + " + " + fragmentPath,
                          "Linking failed:\n" + log);
  }
}

Shader::~Shader() { clean(); }

void Shader::clean() {
  if (mID != 0 && glfwGetCurrentContext()) {
    glDeleteProgram(mID);
    mID = 0;
  }
}

void Shader::use() const { glUseProgram(mID); }

void Shader::setBool(const std::string &name, const bool value) const {
  glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string &name, const int value) const {
  glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string &name, const float value) const {
  glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vector) const {
  glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(vector));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) const {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(matrix));
}

GLint Shader::getUniformLocation(const std::string &name) const {
  if (const auto it = mUniformLocationCache.find(name);
      it != mUniformLocationCache.end())
    return it->second;

  // -1 is cached too: uniforms optimised out by the compiler are silently
  // ignored by glUniform*, and we only want to hear about them once.
  const GLint location = glGetUniformLocation(mID, name.c_str());
  if (location == -1)
    Logger::get()->debug("Uniform '{}' not found in program {}", name, mID);

  mUniformLocationCache[name] = location;
  return location;
}
