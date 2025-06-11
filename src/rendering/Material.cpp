#include "Material.h"

Material::~Material() {
  clean();
}

void Material::clean() {
  for (auto& [key, texture] : m_Textures) {
    if (texture) {
      texture->clean();
    };
  }
}



void Material::bind(const std::shared_ptr<Shader> &shader) const {
  // Bind PBR parameters
  shader->setVec3("uMaterial.albedo", mAlbedo);
  shader->setFloat("uMaterial.metallic", mMetallic);
  shader->setFloat("uMaterial.roughness", mRoughness);
  shader->setFloat("uMaterial.shininess", mShininess);

  // Bind textures
  int textureSlot = 0;
  for (const auto &[type, texture] : m_Textures) {
    texture->bind(textureSlot);

    // Set texture uniforms (e.g., "uMaterial.albedoMap")
    switch (type) {
    case TextureType::Albedo:
      shader->setInt("uMaterial.albedoMap", textureSlot);
      break;
    case TextureType::Specular:
      shader->setInt("uMaterial.specularMap", textureSlot);
      break;
    case TextureType::Normal:
      shader->setInt("uMaterial.normalMap", textureSlot);
      break;
    case TextureType::Metallic:
      shader->setInt("uMaterial.metallicMap", textureSlot);
      break;
    case TextureType::Roughness:
      shader->setInt("uMaterial.roughnessMap", textureSlot);
      break;
    case TextureType::AmbientOcclusion:
      shader->setInt("uMaterial.aoMap", textureSlot);
      break;
    case TextureType::Emissive:
      shader->setInt("uMaterial.emissiveMap", textureSlot);
      break;
    }

    textureSlot++;
  }
}

void Material::setAlbedo(const glm::vec3 &albedo) { mAlbedo = albedo; }

void Material::setMetallic(const float metallic) { mMetallic = metallic; }

void Material::setRoughness(const float roughness) { mRoughness = roughness; }

void Material::setTexture(const TextureType type,
                          const std::shared_ptr<Texture> &texture) {
  m_Textures[type] = texture;
}