#include "Texture.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <algorithm>
#include <cmath>

namespace {
struct PixelFormat {
  GLenum internalFormat;
  GLenum format;
};

PixelFormat pixelFormatFor(const int channels, const bool sRGB) {
  switch (channels) {
  case 1:
    return {GL_R8, GL_RED};
  case 2:
    return {GL_RG8, GL_RG};
  case 3:
    return {static_cast<GLenum>(sRGB ? GL_SRGB8 : GL_RGB8), GL_RGB};
  default:
    return {static_cast<GLenum>(sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8), GL_RGBA};
  }
}
} // namespace

Texture::Texture(const std::string &path, const bool sRGB) {
  // Image rows start at the top, OpenGL expects them to start at the bottom
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data =
      stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, 0);
  if (!data)
    throw ResourceException(path,
                            std::string("stb_image: ") + stbi_failure_reason());

  const auto [internalFormat, format] = pixelFormatFor(mChannels, sRGB);
  const int levels =
      1 + static_cast<int>(std::floor(std::log2(std::max(mWidth, mHeight))));

  // Immutable storage: every mip level is allocated once, up front
  glCreateTextures(GL_TEXTURE_2D, 1, &mID);
  glTextureStorage2D(mID, levels, internalFormat, mWidth, mHeight);

  // Rows of 1 or 3 channel images are not necessarily 4-byte aligned
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTextureSubImage2D(mID, 0, 0, 0, mWidth, mHeight, format, GL_UNSIGNED_BYTE,
                      data);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  // Grayscale maps are expanded so that .rgb reads return the same value
  if (mChannels == 1) {
    const GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
    glTextureParameteriv(mID, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
  }

  glGenerateTextureMipmap(mID);
  glTextureParameteri(mID, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(mID, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTextureParameteri(mID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(mID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  Logger::get()->debug("Loaded texture {} ({}x{}, {} channels)", path, mWidth,
                       mHeight, mChannels);
}

Texture::Texture(const glm::vec4 &color) : mWidth(1), mHeight(1), mChannels(4) {
  const unsigned char texel[] = {
      static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f),
      static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f),
      static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f),
      static_cast<unsigned char>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f)};

  glCreateTextures(GL_TEXTURE_2D, 1, &mID);
  glTextureStorage2D(mID, 1, GL_RGBA8, 1, 1);
  glTextureSubImage2D(mID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, texel);
  glTextureParameteri(mID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(mID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::~Texture() { clean(); }

void Texture::clean() {
  if (mID != 0 && glfwGetCurrentContext()) {
    glDeleteTextures(1, &mID);
    mID = 0;
  }
}

void Texture::bind(const GLuint unit) const { glBindTextureUnit(unit, mID); }
