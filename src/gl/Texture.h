#pragma once
#include <string>
#include "../core/Logger.h"
#include <glad/glad.h>

class Texture {
public:
  explicit Texture(const std::string &path, bool sRGB);
  ~Texture();

  void bind(GLuint slot = 0) const;
  GLuint getID() const { return mID; }

private:
  unsigned int mID;
  int mWidth, mHeight, mChannels;
};


