#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"

#include <memory>

/// Texture maps understood by the forward shader.
enum class TextureType { Albedo, Specular, Normal };

/**
 * Surface description for the Blinn-Phong forward shader.
 *
 * Each map has a fixed texture unit. When a map is missing, a neutral 1x1
 * texture is bound instead and the matching color factor is used, so a
 * material coming from an untextured OBJ still renders with its Kd/Ks colors.
 *
 * Texture unit 0 is left free for the shadow map.
 */
class Material {
public:
  static constexpr int kAlbedoUnit = 1;
  static constexpr int kSpecularUnit = 2;
  static constexpr int kNormalUnit = 3;

  /// Binds every map and factor to the `uMaterial` uniform struct.
  void bind(const Shader &shader) const;

  /// Binds only the albedo map, for passes that just need alpha testing.
  void bindAlbedo(const Shader &shader, const std::string &uniform,
                  int unit) const;

  /// Multiplies the albedo map (linear RGB).
  void setAlbedo(const glm::vec3 &albedo) { mAlbedo = albedo; }

  /// Multiplies the specular map (linear RGB).
  void setSpecular(const glm::vec3 &specular) { mSpecular = specular; }

  /// Blinn-Phong exponent.
  void setShininess(const float shininess) { mShininess = shininess; }

  void setTexture(TextureType type, const std::shared_ptr<Texture> &texture);
  [[nodiscard]] bool hasTexture(TextureType type) const;

  /// Frees the shared fallback textures. Must run before the GL context dies.
  static void releaseDefaultTextures();

private:
  glm::vec3 mAlbedo{1.0f};
  glm::vec3 mSpecular{0.5f};
  float mShininess = 32.0f;

  std::shared_ptr<Texture> mAlbedoMap;
  std::shared_ptr<Texture> mSpecularMap;
  std::shared_ptr<Texture> mNormalMap;
};
