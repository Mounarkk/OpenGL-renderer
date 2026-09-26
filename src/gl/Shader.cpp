#include "Shader.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"

#include <GLFW/glfw3.h>
#include <gtc/type_ptr.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

namespace {
constexpr int kMaxIncludeDepth = 16;

std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw ShaderException(path, "cannot open the file");

  std::stringstream stream;
  stream << file.rdbuf();
  return stream.str();
}

/**
 * Expands `#include "file"` lines recursively.
 *
 * Each file gets a GLSL source string number (its index in `files`), set
 * with `#line`, so that driver errors read "<file index>:<line>". The table
 * is printed when compilation fails.
 */
std::string expandIncludes(const std::string &path,
                           std::vector<std::string> &files,
                           std::set<std::string> &included, const int depth) {
  if (depth > kMaxIncludeDepth)
    throw ShaderException(path, "includes nested too deeply");

  const int fileIndex = static_cast<int>(files.size());
  files.push_back(path);
  const std::filesystem::path directory =
      std::filesystem::path(path).parent_path();

  std::istringstream source(readFile(path));
  std::ostringstream output;
  std::string line;
  int lineNumber = 0;
  while (std::getline(source, line)) {
    ++lineNumber;
    const size_t directive = line.find_first_not_of(" \t");
    if (directive == std::string::npos ||
        line.compare(directive, 8, "#include") != 0) {
      output << line << '\n';
      continue;
    }

    const size_t open = line.find('"');
    const size_t close = line.find('"', open + 1);
    if (open == std::string::npos || close == std::string::npos)
      throw ShaderException(path, "malformed #include at line " +
                                      std::to_string(lineNumber));

    const std::string includePath =
        (directory / line.substr(open + 1, close - open - 1))
            .lexically_normal()
            .string();
    if (included.insert(includePath).second)
      output << expandIncludes(includePath, files, included, depth + 1);
    // Back to the numbering of this file
    output << "#line " << lineNumber + 1 << ' ' << fileIndex << '\n';
  }
  return output.str();
}

/// Loads a stage with its includes and inserts the defines after #version.
std::string preprocess(const std::string &path, const std::string &defines,
                       std::vector<std::string> &files) {
  std::set<std::string> included;
  std::string source = expandIncludes(path, files, included, 0);

  const size_t version = source.find("#version");
  if (version == std::string::npos)
    throw ShaderException(path, "missing #version directive");
  const size_t versionEnd = source.find('\n', version);
  // The line following #version is line 2 of the main file (string 0)
  source.insert(versionEnd + 1, defines + "#line 2 0\n");
  return source;
}

std::string fileTable(const std::vector<std::string> &files) {
  std::string table;
  for (size_t i = 0; i < files.size(); ++i)
    table += "\n  " + std::to_string(i) + ": " + files[i];
  return table;
}

GLuint compileStage(const GLenum stage, const std::string &path,
                    const std::string &defines) {
  std::vector<std::string> files;
  const std::string source = preprocess(path, defines, files);

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
    throw ShaderException(path, "compilation failed:\n" + log +
                                    "Source files:" + fileTable(files));
  }
  return shader;
}
} // namespace

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath,
               const std::string &geometryPath, const std::string &defines) {
  std::vector<GLuint> stages;
  auto deleteStages = [&stages]() {
    for (const GLuint stage : stages)
      glDeleteShader(stage);
  };

  try {
    stages.push_back(compileStage(GL_VERTEX_SHADER, vertexPath, defines));
    stages.push_back(compileStage(GL_FRAGMENT_SHADER, fragmentPath, defines));
    if (!geometryPath.empty())
      stages.push_back(compileStage(GL_GEOMETRY_SHADER, geometryPath, defines));
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
                          "linking failed:\n" + log);
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
  glProgramUniform1i(mID, getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string &name, const int value) const {
  glProgramUniform1i(mID, getUniformLocation(name), value);
}

void Shader::setUInt(const std::string &name, const unsigned int value) const {
  glProgramUniform1ui(mID, getUniformLocation(name), value);
}

void Shader::setFloat(const std::string &name, const float value) const {
  glProgramUniform1f(mID, getUniformLocation(name), value);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vector) const {
  glProgramUniform3fv(mID, getUniformLocation(name), 1, glm::value_ptr(vector));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) const {
  glProgramUniformMatrix4fv(mID, getUniformLocation(name), 1, GL_FALSE,
                            glm::value_ptr(matrix));
}

GLint Shader::getUniformLocation(const std::string &name) const {
  if (const auto it = mUniformLocationCache.find(name);
      it != mUniformLocationCache.end())
    return it->second;

  // -1 is cached too: uniforms optimised out by the compiler are silently
  // ignored by glProgramUniform*, and we only want to hear about them once.
  const GLint location = glGetUniformLocation(mID, name.c_str());
  if (location == -1)
    Logger::get()->debug("Uniform '{}' not found in program {}", name, mID);

  mUniformLocationCache[name] = location;
  return location;
}
