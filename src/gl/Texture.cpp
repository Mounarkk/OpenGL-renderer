#include "Texture.h"
#include "../../vendor/stb_image/stb_image.h"
#include "../core/RendererException.h"
#include "GLFW/glfw3.h"

Texture::Texture(const std::string &path, const bool sRGB) {
  // Load image data using stb_image library
  stbi_set_flip_vertically_on_load(
      true); // OpenGL expects (0,0) at bottom-left, images have (0,0) at
             // top-left
  unsigned char *data = stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels,
                                  0); // Load with original channel count
  if (!data) {
    Logger::get()->error("Failed to load texture: {}", path);
    throw ResourceException(path, "STB image loading failed: " +
                                      std::string(stbi_failure_reason()));
  }

  // Determine OpenGL texture formats based on channel count and color space
  GLenum internalFormat = GL_RGB8; // Default to RGB
  GLenum format = GL_RGB;
  if (mChannels == 4) {
    // RGBA texture with optional sRGB gamma correction
    internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    format = GL_RGBA;
  } else if (mChannels == 3) {
    // RGB texture with optional sRGB gamma correction
    internalFormat = sRGB ? GL_SRGB8 : GL_RGB8;
    format = GL_RGB;
  }

  // Create and configure OpenGL texture object
  glGenTextures(1, &mID);            // Generate texture ID
  glBindTexture(GL_TEXTURE_2D, mID); // Bind for configuration
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0,
               format, // Upload image data to GPU
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(
      GL_TEXTURE_2D); // Generate mipmaps for better quality at distance

  // Configure texture sampling parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_REPEAT); // Repeat texture horizontally
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  GL_REPEAT); // Repeat texture vertically
  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_MIN_FILTER, // Use trilinear filtering when minifying
      GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR); // Use linear filtering when magnifying

  stbi_image_free(data); // Free CPU memory (data is now on GPU)
  Logger::get()->info("Loaded texture: {}", path);
}

Texture::~Texture() { clean(); }

void Texture::clean() {
  if (mID != 0 && glfwGetCurrentContext()) {
    Logger::get()->info("Deleted texture ID: {}", mID);
    glDeleteTextures(1, &mID);
    mID = 0;
  }
}

void Texture::bind(const GLuint slot) const {
  glActiveTexture(GL_TEXTURE0 +
                  slot); // Activate the specified texture unit (0-31)
  glBindTexture(GL_TEXTURE_2D, mID); // Bind this texture to the active unit
}
