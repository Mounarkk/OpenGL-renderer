#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "Renderer.h"

class Texture {
public:
Texture(const std::string & filePath, int format);
~Texture();

void bind(unsigned int slot = 0) const;
void unbind() const;

[[nodiscard]] int getWidth() const { return mWidth; };
[[nodiscard]] int getHeight() const { return mHeight; };

private:
  unsigned int mRendererId;
  std::string mFileName;
  unsigned char* mData;
  int mWidth, mHeight, mBpp;
};



#endif //TEXTURE_H
