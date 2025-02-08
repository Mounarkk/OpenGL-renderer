#ifndef TEXTURE_H
#define TEXTURE_H

#include "Renderer.h"
#include <string>

class Texture {
public:
  explicit Texture(const std::string &filePath);
  ~Texture();

  void bind(unsigned int slot = 0) const;
  void unbind() const;

  [[nodiscard]] int getWidth() const { return mWidth; };
  [[nodiscard]] int getHeight() const { return mHeight; };
  [[nodiscard]] unsigned int getRendererId() const { return mRendererId; };

private:
  unsigned int mRendererId;
  std::string mFileName;
  unsigned char *mData;
  int mWidth, mHeight, mBpp;
};

#endif // TEXTURE_H
