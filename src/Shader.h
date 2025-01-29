#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <fstream>
#include <fwd.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  GLuint mRendererId;

  Shader(const char *vertexPath, const char *fragmentPath);
  void use() const;
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, float value1, float value2,
               float value3) const;
  void setMat4(const std::string &name, const glm::mat4 &matrix) const;
};

#endif // SHADER_H
