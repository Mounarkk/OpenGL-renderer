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

class Shader {
public:
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  ~Shader();

  void use() const;

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, float value1, float value2,
               float value3) const;
  void setMat4(const std::string &name, const glm::mat4 &matrix) const;

private:
  GLuint mID;
  std::unordered_map<std::string, GLint> mUniformLocationCache;

  GLint getUniformLocation(const std::string &name);
};
