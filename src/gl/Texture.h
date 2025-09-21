#pragma once
#include "../core/Logger.h"
#include <glad/glad.h>
#include <string>

/**
 * OpenGL texture wrapper for loading and managing 2D textures.
 * 
 * The Texture class handles loading images from files and creating OpenGL
 * texture objects. It supports common image formats through stb_image and
 * provides automatic texture parameter setup for typical use cases. The class
 * handles both sRGB and linear color spaces and manages texture binding for
 * shader usage.
 * 
 * Key features:
 * - Automatic image loading from common formats (PNG, JPG, etc.)
 * - sRGB color space support for proper gamma correction
 * - Texture unit binding for multi-texture rendering
 * - Automatic mipmap generation and filtering setup
 * - RAII resource management
 */
class Texture {
public:
  /**
   * Loads a texture from an image file.
   * @param path File path to the image (supports PNG, JPG, TGA, BMP, etc.)
   * @param sRGB Whether to treat the image as sRGB color space (true for diffuse textures)
   */
  explicit Texture(const std::string &path, bool sRGB);
  
  /**
   * Destructor that automatically cleans up the OpenGL texture.
   */
  ~Texture();
  
  /**
   * Manually deletes the OpenGL texture object.
   * Called automatically by destructor.
   */
  void clean();

  /**
   * Binds this texture to the specified texture unit for rendering.
   * @param slot Texture unit to bind to (0-31, defaults to 0)
   */
  void bind(GLuint slot = 0) const;
  
  /**
   * Gets the OpenGL texture ID for advanced operations.
   * @return OpenGL texture object ID
   */
  [[nodiscard]] GLuint getID() const { return mID; }

private:
  unsigned int mID;
  int mWidth, mHeight, mChannels;
};
