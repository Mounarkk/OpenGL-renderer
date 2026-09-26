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

void bindMap(const std::shared_ptr<Texture> &map,
             const std::shared_ptr<Texture> &fallback, const GLuint unit) {
  (map ? map : fallback)->bind(unit);
}
} // namespace

void Material::bind(const Shader &shader) const {
  // The samplers declare these units with layout(binding), nothing to set
  bindMap(mAlbedoMap, whiteTexture(), ShaderInterface::kAlbedoUnit);
  bindMap(mSpecularMap, whiteTexture(), ShaderInterface::kSpecularUnit);
  bindMap(mNormalMap, flatNormalTexture(), ShaderInterface::kNormalUnit);

  shader.setVec3("uMaterial.albedo", mAlbedo);
  shader.setVec3("uMaterial.specular", mSpecular);
  shader.setFloat("uMaterial.shininess", mShininess);
  shader.setBool("uMaterial.hasNormalMap", mNormalMap != nullptr);
}

void Material::bindAlbedo() const {
  bindMap(mAlbedoMap, whiteTexture(), ShaderInterface::kAlbedoUnit);
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
