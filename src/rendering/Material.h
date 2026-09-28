#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include "ShaderInterface.h"

#include <array>
#include <memory>

/// Texture maps understood by the forward shader.
enum class TextureType {
  Albedo,            ///< sRGB base color, alpha used for cut-outs
  Normal,            ///< Tangent space normal map
  MetallicRoughness, ///< glTF packing: roughness in G, metallic in B
  Occlusion,         ///< Ambient occlusion in R
  Emissive,          ///< sRGB emitted color
  Specular,          ///< Dielectric reflectance, for converted Phong maps
  Count
};

/// How the albedo alpha is used.
enum class AlphaMode {
  Opaque, ///< Alpha ignored
  Mask    ///< Pixels below the cutoff are discarded (foliage, fences)
};

/**
 * Metallic-roughness surface description, as in glTF 2.0.
 *
 * Every map is multiplied by its factor. A missing map is replaced by a
 * neutral 1x1 texture (white, or a flat normal) so that the factors alone
 * describe the surface: an untextured OBJ renders with its Kd color.
 *
 * Each map has a fixed texture unit, see ShaderInterface.
 */
class Material {
public:
  /// Binds every map to its unit and sets the `uMaterial` factors.
  void bind(const Shader &shader) const;

  /// Binds the albedo map and sets `uAlphaCutoff`, for passes that only need
  /// alpha testing.
  void bindAlbedo(const Shader &shader) const;

  /// Linear base color and alpha.
  void setAlbedo(const glm::vec4 &albedo) { mAlbedo = albedo; }
  void setAlbedo(const glm::vec3 &albedo) { mAlbedo = glm::vec4(albedo, 1.0f); }

  /// 0 for dielectrics (plastic, wood, stone), 1 for bare metals.
  void setMetallic(const float metallic) { mMetallic = metallic; }

  /// 0 is a perfect mirror, 1 is fully diffuse-looking.
  void setRoughness(const float roughness) { mRoughness = roughness; }

  /// Reflectance of dielectrics at normal incidence, remapped so that the
  /// default 0.5 gives the usual 4% (F0 = 0.08 * specular).
  void setSpecular(const float specular) { mSpecular = specular; }

  /// Linear emitted radiance, added on top of the lighting.
  void setEmissive(const glm::vec3 &emissive) { mEmissive = emissive; }

  void setAlphaMode(AlphaMode mode, float cutoff = 0.5f);

  void setTexture(TextureType type, const std::shared_ptr<Texture> &texture);
  [[nodiscard]] bool hasTexture(TextureType type) const;

  /// Frees the shared fallback textures. Must run before the GL context dies.
  static void releaseDefaultTextures();

private:
  glm::vec4 mAlbedo{1.0f};
  float mMetallic = 0.0f;
  float mRoughness = 0.5f;
  float mSpecular = 0.5f;
  glm::vec3 mEmissive{0.0f};
  // Cut-out by default: OBJ files have no alpha mode but often rely on it
  AlphaMode mAlphaMode = AlphaMode::Mask;
  float mAlphaCutoff = 0.5f;

  std::array<std::shared_ptr<Texture>, static_cast<size_t>(TextureType::Count)>
      mMaps;

  [[nodiscard]] const std::shared_ptr<Texture> &map(TextureType type) const {
    return mMaps[static_cast<size_t>(type)];
  }
  [[nodiscard]] float alphaCutoff() const {
    // A negative cutoff never discards
    return mAlphaMode == AlphaMode::Mask ? mAlphaCutoff : -1.0f;
  }
};
