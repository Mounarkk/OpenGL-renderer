#pragma once
#include "../core/Logger.h"
#include <glad/glad.h>
#include <string>

class Texture {
public:
  explicit Texture(const std::string &path, bool sRGB);
  ~Texture();

  void bind(GLuint slot = 0) const;
  [[nodiscard]] GLuint getID() const { return mID; }

private:
  unsigned int mID;
  int mWidth, mHeight, mChannels;
};
