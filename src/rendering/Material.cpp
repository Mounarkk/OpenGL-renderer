#include "Material.h"

namespace {
std::shared_ptr<Texture> &whiteTexture() {
  static std::shared_ptr<Texture> texture;
  if (!texture)
    texture = std::make_shared<Texture>(glm::vec4(1.0f));
  return texture;
}

// Tangent space "straight up" normal, i.e. (0, 0, 1) encoded in [0, 1]
std::shared_ptr<Texture> &flatNormalTexture() {
  static std::shared_ptr<Texture> texture;
  if (!texture)
    texture = std::make_shared<Texture>(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
  return texture;
}

void bindMap(const Shader &shader, const std::string &uniform,
             const std::shared_ptr<Texture> &map,
             const std::shared_ptr<Texture> &fallback, const int unit) {
  (map ? map : fallback)->bind(unit);
  shader.setInt(uniform, unit);
}
} // namespace

void Material::bind(const Shader &shader) const {
  bindMap(shader, "uMaterial.albedoMap", mAlbedoMap, whiteTexture(),
          kAlbedoUnit);
  bindMap(shader, "uMaterial.specularMap", mSpecularMap, whiteTexture(),
          kSpecularUnit);
  bindMap(shader, "uMaterial.normalMap", mNormalMap, flatNormalTexture(),
          kNormalUnit);

  shader.setVec3("uMaterial.albedo", mAlbedo);
  shader.setVec3("uMaterial.specular", mSpecular);
  shader.setFloat("uMaterial.shininess", mShininess);
  shader.setBool("uMaterial.hasNormalMap", mNormalMap != nullptr);
}

void Material::bindAlbedo(const Shader &shader, const std::string &uniform,
                          const int unit) const {
  bindMap(shader, uniform, mAlbedoMap, whiteTexture(), unit);
}

void Material::setTexture(const TextureType type,
                          const std::shared_ptr<Texture> &texture) {
  switch (type) {
  case TextureType::Albedo:
    mAlbedoMap = texture;
    break;
  case TextureType::Specular:
    mSpecularMap = texture;
    break;
  case TextureType::Normal:
    mNormalMap = texture;
    break;
  }
}

bool Material::hasTexture(const TextureType type) const {
  switch (type) {
  case TextureType::Albedo:
    return mAlbedoMap != nullptr;
  case TextureType::Specular:
    return mSpecularMap != nullptr;
  case TextureType::Normal:
    return mNormalMap != nullptr;
  }
  return false;
}

void Material::releaseDefaultTextures() {
  whiteTexture().reset();
  flatNormalTexture().reset();
}
