#include "Shader.h"

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  // Get the vertex/fragment source code from filePath
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  // Check if stream objects can throw exceptions
  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vShaderFile.close();
    fShaderFile.close();
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  } catch ([[maybe_unused]] std::ifstream::failure &e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n" << std::endl;
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  // ------------------------------------------------------------------------
  // Create the vertex shader and compile it
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
  glCompileShader(vertexShader);

  // Check the vertex shader compilation
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // ------------------------------------------------------------------------
  // Create the fragment shader and compile it
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
  glCompileShader(fragmentShader);

  // Check the fragment shader compilation
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // ------------------------------------------------------------------------
  // Create the shader program that links all the shaders
  GLuint shaderProgram = glCreateProgram();
  this->mRendererId = shaderProgram;
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Check that linking what successful
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Shader::use() const { glUseProgram(this->mRendererId); }

void Shader::setBool(const std::string &name, const bool value) const {
  glUniform1i(glGetUniformLocation(this->mRendererId, name.c_str()),
              static_cast<int>(value));
}

void Shader::setInt(const std::string &name, const int value) const {
  glUniform1i(glGetUniformLocation(this->mRendererId, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, const float value) const {
  glUniform1f(glGetUniformLocation(this->mRendererId, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, const float value1,
                     const float value2, const float value3) const {
  glUniform3f(glGetUniformLocation(this->mRendererId, name.c_str()), value1,
              value2, value3);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(this->mRendererId, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}
