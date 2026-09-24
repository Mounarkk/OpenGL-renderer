#pragma once
#include <glad/glad.h>
#include <glm.hpp>

#include <string>

/**
 * 2D texture loaded from an image file with stb_image, or created as a single
 * texel of constant color (used as a neutral default for missing maps).
 *
 * Mipmaps are generated and sampling is trilinear with repeat wrapping.
 */
class Texture {
public:
  /**
   * Loads an image file. Throws a ResourceException if it cannot be read.
   * @param path Image path (PNG, JPG, TGA, BMP...)
   * @param sRGB True for color data (albedo), false for data maps (normals,
   * specular...). Only applies to 3 and 4 channel images.
   */
  Texture(const std::string &path, bool sRGB);

  /// Creates a 1x1 texture of the given linear RGBA color.
  explicit Texture(const glm::vec4 &color);

  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  /// Deletes the GL texture. Safe to call several times.
  void clean();

  void bind(GLuint slot = 0) const;

  [[nodiscard]] GLuint getID() const { return mID; }
  [[nodiscard]] int getChannels() const { return mChannels; }

private:
  GLuint mID = 0;
  int mWidth = 0;
  int mHeight = 0;
  int mChannels = 0;
};
