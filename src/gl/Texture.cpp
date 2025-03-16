#include "Texture.h"
#include "../../vendor/stb_image/stb_image.h"

Texture::Texture(const std::string &path, const bool sRGB) {
  // Load image data
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data =
      stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, 0);
  if (!data) {
    Logger::get()->error("Failed to load texture: {}", path);
    return;
  }

  // Determine format
  GLenum internalFormat = GL_RGB8;
  GLenum format = GL_RGB;
  if (mChannels == 4) {
    internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    format = GL_RGBA;
  } else if (mChannels == 3) {
    internalFormat = sRGB ? GL_SRGB8 : GL_RGB8;
    format = GL_RGB;
  }

  // Create OpenGL texture
  glGenTextures(1, &mID);
  glBindTexture(GL_TEXTURE_2D, mID);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Set parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  Logger::get()->info("Loaded texture: {}", path);
}

Texture::~Texture() {
  if (mID != 0) {
    glDeleteTextures(1, &mID);
    Logger::get()->info("Deleted texture ID: {}", mID);
  }
}

void Texture::bind(const GLuint slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, mID);
}
