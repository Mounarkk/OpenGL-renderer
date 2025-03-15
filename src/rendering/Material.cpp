#include "Material.h"

Material::Material(std::shared_ptr<Shader> shader)
    : m_Shader(shader) {}

void Material::bind() const {
  m_Shader->use();

  // Bind PBR parameters
  m_Shader->setVec3("uMaterial.albedo", m_Albedo.x, m_Albedo.y, m_Albedo.z);
  m_Shader->setFloat("uMaterial.metallic", m_Metallic);
  m_Shader->setFloat("uMaterial.roughness", m_Roughness);

  // Bind textures
  int textureSlot = 0;
  for (const auto& [type, texture] : m_Textures) {
    texture->bind(textureSlot);

    // Set texture uniforms (e.g., "uMaterial.albedoMap")
    switch (type) {
    case TextureType::Albedo:
      m_Shader->setInt("uMaterial.albedoMap", textureSlot);
      break;
    case TextureType::Normal:
      m_Shader->setInt("uMaterial.normalMap", textureSlot);
      break;
    case TextureType::Metallic:
      m_Shader->setInt("uMaterial.metallicMap", textureSlot);
      break;
    case TextureType::Roughness:
      m_Shader->setInt("uMaterial.roughnessMap", textureSlot);
      break;
    case TextureType::AmbientOcclusion:
      m_Shader->setInt("uMaterial.aoMap", textureSlot);
      break;
    case TextureType::Emissive:
      m_Shader->setInt("uMaterial.emissiveMap", textureSlot);
      break;
    }

    textureSlot++;
  }
}

void Material::setAlbedo(const glm::vec3& albedo) {
  m_Albedo = albedo;
}

void Material::setMetallic(float metallic) {
  m_Metallic = metallic;
}

void Material::setRoughness(float roughness) {
  m_Roughness = roughness;
}

void Material::setTexture(TextureType type, std::shared_ptr<Texture> texture) {
  m_Textures[type] = texture;
}