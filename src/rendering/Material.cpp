#include "Material.h"

using namespace ShaderInterface;

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
  bindMap(map(TextureType::Albedo), whiteTexture(), kAlbedoUnit);
  bindMap(map(TextureType::Normal), flatNormalTexture(), kNormalUnit);
  bindMap(map(TextureType::MetallicRoughness), whiteTexture(),
          kMetallicRoughnessUnit);
  bindMap(map(TextureType::Occlusion), whiteTexture(), kOcclusionUnit);
  bindMap(map(TextureType::Emissive), whiteTexture(), kEmissiveUnit);
  bindMap(map(TextureType::Specular), whiteTexture(), kSpecularUnit);

  shader.setVec4("uMaterial.albedo", mAlbedo);
  shader.setFloat("uMaterial.metallic", mMetallic);
  shader.setFloat("uMaterial.roughness", mRoughness);
  shader.setFloat("uMaterial.specular", mSpecular);
  shader.setVec3("uMaterial.emissive", mEmissive);
  shader.setFloat("uMaterial.alphaCutoff", alphaCutoff());
  shader.setBool("uMaterial.hasNormalMap", hasTexture(TextureType::Normal));
}

void Material::bindAlbedo(const Shader &shader) const {
  bindMap(map(TextureType::Albedo), whiteTexture(), kAlbedoUnit);
  shader.setFloat("uAlphaCutoff", alphaCutoff());
}

void Material::setAlphaMode(const AlphaMode mode, const float cutoff) {
  mAlphaMode = mode;
  mAlphaCutoff = cutoff;
}

void Material::setTexture(const TextureType type,
                          const std::shared_ptr<Texture> &texture) {
  mMaps[static_cast<size_t>(type)] = texture;
}

bool Material::hasTexture(const TextureType type) const {
  return map(type) != nullptr;
}

void Material::releaseDefaultTextures() {
  whiteTexture().reset();
  flatNormalTexture().reset();
}
