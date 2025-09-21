#include "Shader.h"
#include "../core/RendererException.h"

#include "GLFW/glfw3.h"

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
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
    Logger::get()->error("Failed to open shader files: {}, {}", vertexPath,
                         fragmentPath);
    throw ShaderException("file loading", "Failed to open shader files: " + vertexPath + ", " + fragmentPath);
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  // ------------------------------------------------------------------------
  // Create the vertex shader and compile it
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);  // Create empty vertex shader object
  glShaderSource(vertexShader, 1, &vShaderCode, nullptr); // Attach source code to shader
  glCompileShader(vertexShader);                           // Compile the shader source into GPU code

  // Check the vertex shader compilation status
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);  // Query compilation status
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);  // Get detailed error message
    Logger::get()->error("Vertex shader compilation failed: {}", infoLog);
    glDeleteShader(vertexShader);  // Clean up on failure
    throw ShaderException("vertex", "Compilation failed: " + std::string(infoLog));
  }

  // ------------------------------------------------------------------------
  // Create the fragment shader and compile it
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);  // Create empty fragment shader object
  glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);    // Attach source code to shader
  glCompileShader(fragmentShader);                             // Compile the shader source into GPU code

  // Check the fragment shader compilation status
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);  // Query compilation status
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);  // Get detailed error message
    Logger::get()->error("Fragment shader compilation failed: {}", infoLog);
    glDeleteShader(vertexShader);    // Clean up on failure
    glDeleteShader(fragmentShader);  // Clean up on failure
    throw ShaderException("fragment", "Compilation failed: " + std::string(infoLog));
  }

  // ------------------------------------------------------------------------
  // Create the shader program that links all the shaders together
  GLuint shaderProgram = glCreateProgram();                    // Create empty program object
  this->mID = shaderProgram;
  glAttachShader(shaderProgram, vertexShader);                 // Attach vertex shader to program
  glAttachShader(shaderProgram, fragmentShader);               // Attach fragment shader to program
  glLinkProgram(shaderProgram);                                // Link shaders into final program

  // Check that linking was successful
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);     // Query linking status
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Get detailed error message
    Logger::get()->error("Shader program linking failed: {}", infoLog);
    glDeleteShader(vertexShader);    // Clean up on failure
    glDeleteShader(fragmentShader);  // Clean up on failure
    glDeleteProgram(shaderProgram);  // Clean up on failure
    throw ShaderException("program linking", "Linking failed: " + std::string(infoLog));
  }

  // Clean up individual shader objects (no longer needed after linking)
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() {
  clean();
}

void Shader::clean() {
  if (mID != 0 && glfwGetCurrentContext()) {
    Logger::get()->info("Deleted shader program ID: {}", mID);
    glDeleteProgram(mID);
    mID = 0;
  }
}


void Shader::use() const { glUseProgram(this->mID); }

void Shader::setBool(const std::string &name, const bool value) const {
  glUniform1i(glGetUniformLocation(this->mID, name.c_str()),
              static_cast<int>(value));
}

void Shader::setInt(const std::string &name, const int value) const {
  glUniform1i(glGetUniformLocation(this->mID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, const float value) const {
  glUniform1f(glGetUniformLocation(this->mID, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, const glm::vec3 vector) const {
  glUniform3f(glGetUniformLocation(this->mID, name.c_str()), vector.x, vector.y,
              vector.z);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) const {
  // Upload 4x4 matrix to GPU: count=1, transpose=false, data=matrix pointer
  glUniformMatrix4fv(glGetUniformLocation(this->mID, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(matrix));
}

GLint Shader::getUniformLocation(const std::string &name) {
  // Check cache first to avoid expensive OpenGL calls
  if (mUniformLocationCache.find(name) != mUniformLocationCache.end())
    return mUniformLocationCache[name];

  // Query OpenGL for uniform location (expensive operation)
  const GLint location = glGetUniformLocation(mID, name.c_str());
  if (location == -1)
    Logger::get()->warn("Uniform '{}' not found in shader program", name);

  // Cache the result for future use
  mUniformLocationCache[name] = location;
  return location;
}
