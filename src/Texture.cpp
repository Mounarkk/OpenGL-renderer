#include "Texture.h"

#include "../vendor/stb_image/stb_image.h"
#include <iostream>

Texture::Texture(const std::string &filePath)
    : mRendererId(0), mFileName(filePath), mData(nullptr), mWidth(0),
      mHeight(0), mBpp(0) {
  // load and create a texture
  glGenTextures(1, &mRendererId);
  glBindTexture(GL_TEXTURE_2D, mRendererId);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // load image, create texture and generate mipmaps
  stbi_set_flip_vertically_on_load(true);

  mData = stbi_load(filePath.c_str(), &mWidth, &mHeight, &mBpp, 0);
  if (mData) {
    GLenum format;
    if (mBpp == 1) {
      format = GL_RED;
    } else if (mBpp == 3) {
      format = GL_RGB;
    } else if (mBpp == 4) {
      format = GL_RGBA;
    }

    // Will generate a texture on the currently bound texture object
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, format,
                 GL_UNSIGNED_BYTE, mData);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(mData);
}

Texture::~Texture() { glDeleteTextures(1, &mRendererId); }

void Texture::bind(const unsigned int slot /* From 0 to 31 slots */) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, mRendererId);
}
