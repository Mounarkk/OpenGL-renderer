#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include <memory>
#include <unordered_map>

enum class TextureType {
  Albedo,
  Specular,
  Normal,
  Metallic,
  Roughness,
  AmbientOcclusion,
  Emissive
};

class Material {
public:
  Material() = default;
  ~Material();
  void clean();

  void bind(const std::shared_ptr<Shader> &shader) const;

  // Set PBR parameters
  void setAlbedo(const glm::vec3 &albedo);
  void setMetallic(float metallic);
  void setRoughness(float roughness);

  // Set textures
  void setTexture(TextureType type, const std::shared_ptr<Texture> &texture);
private:
  glm::vec3 mAlbedo = {1.0f, 1.0f, 1.0f}; // Base color
  float mMetallic = 0.0f;  // Metallic factor (0 = dielectric, 1 = metal)
  float mRoughness = 0.5f; // Roughness factor (0 = smooth, 1 = rough)
  float mShininess = 64.f;

  std::unordered_map<TextureType, std::shared_ptr<Texture>> m_Textures;
};
