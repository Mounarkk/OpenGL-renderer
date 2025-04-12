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
  explicit Material(std::shared_ptr<Shader> shader);
  ~Material();
  void clean();

  void bind() const;

  // Set PBR parameters
  void setAlbedo(const glm::vec3 &albedo);
  void setMetallic(float metallic);
  void setRoughness(float roughness);

  // Set textures
  void setTexture(TextureType type, const std::shared_ptr<Texture> &texture);

  std::shared_ptr<Shader> getShader() const;

private:
  std::shared_ptr<Shader> m_Shader;
  glm::vec3 m_Albedo = {1.0f, 1.0f, 1.0f}; // Base color
  float m_Metallic = 0.0f;  // Metallic factor (0 = dielectric, 1 = metal)
  float m_Roughness = 0.5f; // Roughness factor (0 = smooth, 1 = rough)

  std::unordered_map<TextureType, std::shared_ptr<Texture>> m_Textures;
};
