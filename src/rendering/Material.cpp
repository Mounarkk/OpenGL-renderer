#include "Material.h"

Material::Material(const std::shared_ptr<Shader> &shader) : mShader(shader) {}

Material::~Material() {
  clean();
}

void Material::clean() {
  if (mShader) {
    mShader->clean();
  }

  for (auto& [key, texture] : m_Textures) {
    if (texture) {
      texture->clean();
    };
  }
}



void Material::bind() const {
  mShader->use();

  // Bind PBR parameters
  mShader->setVec3("uMaterial.albedo", mAlbedo);
  mShader->setFloat("uMaterial.metallic", mMetallic);
  mShader->setFloat("uMaterial.roughness", mRoughness);
  mShader->setFloat("uMaterial.shininess", mShininess);

  // Bind textures
  int textureSlot = 0;
  for (const auto &[type, texture] : m_Textures) {
    texture->bind(textureSlot);

    // Set texture uniforms (e.g., "uMaterial.albedoMap")
    switch (type) {
    case TextureType::Albedo:
      mShader->setInt("uMaterial.albedoMap", textureSlot);
      break;
    case TextureType::Specular:
      mShader->setInt("uMaterial.specularMap", textureSlot);
      break;
    case TextureType::Normal:
      mShader->setInt("uMaterial.normalMap", textureSlot);
      break;
    case TextureType::Metallic:
      mShader->setInt("uMaterial.metallicMap", textureSlot);
      break;
    case TextureType::Roughness:
      mShader->setInt("uMaterial.roughnessMap", textureSlot);
      break;
    case TextureType::AmbientOcclusion:
      mShader->setInt("uMaterial.aoMap", textureSlot);
      break;
    case TextureType::Emissive:
      mShader->setInt("uMaterial.emissiveMap", textureSlot);
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

std::shared_ptr<Shader> Material::getShader() const { return mShader; }