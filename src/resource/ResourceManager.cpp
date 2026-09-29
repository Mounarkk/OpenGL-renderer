#include "ResourceManager.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "../rendering/ShaderInterface.h"
#include "../scene/Scene.h"
#include "ModelLoader.h"

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>

#include <algorithm>
#include <cmath>

std::unordered_map<std::string, std::weak_ptr<Texture>>
    ResourceManager::sTextureCache;
std::unordered_map<std::string, std::weak_ptr<Shader>>
    ResourceManager::sShaderCache;
std::unordered_map<std::string, std::shared_ptr<Model>>
    ResourceManager::sModelCache;

std::shared_ptr<Texture> ResourceManager::loadTexture(const std::string &path,
                                                      const bool sRGB) {
  // The same image may be used as color and as data, hence the flag in the key
  const std::string key = path + (sRGB ? "|srgb" : "|linear");
  if (const auto it = sTextureCache.find(key); it != sTextureCache.end())
    if (auto texture = it->second.lock())
      return texture;

  auto texture = std::make_shared<Texture>(path, sRGB);
  sTextureCache[key] = texture;
  return texture;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string &vsPath,
                                                    const std::string &fsPath,
                                                    const std::string &gsPath) {
  const std::string key = vsPath + "|" + fsPath + "|" + gsPath;
  if (const auto it = sShaderCache.find(key); it != sShaderCache.end())
    if (auto shader = it->second.lock())
      return shader;

  auto shader = std::make_shared<Shader>(vsPath, fsPath, gsPath,
                                         ShaderInterface::defines());
  sShaderCache[key] = shader;
  Logger::get()->debug("Compiled shader {} | {} | {}", vsPath, fsPath, gsPath);
  return shader;
}

std::shared_ptr<Texture> ResourceManager::loadMaterialTexture(
    const aiMaterial &material, const aiTextureType type,
    const std::string &directory, const bool sRGB) {
  if (material.GetTextureCount(type) == 0)
    return nullptr;

  aiString name;
  material.GetTexture(type, 0, &name);
  std::string relativePath = name.C_Str();

  if (!relativePath.empty() && relativePath[0] == '*') {
    Logger::get()->warn("Embedded texture {} in material {} is not supported",
                        relativePath, material.GetName().C_Str());
    return nullptr;
  }

  // Files exported on Windows often use backslashes
  std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

  try {
    return loadTexture(directory + relativePath, sRGB);
  } catch (const RendererException &e) {
    Logger::get()->warn("{}", e.what());
    return nullptr;
  }
}

std::shared_ptr<Material>
ResourceManager::loadMaterial(const aiMaterial &source,
                              const std::string &directory) {
  auto material = std::make_shared<Material>();

  if (auto albedo = loadMaterialTexture(source, aiTextureType_BASE_COLOR,
                                        directory, true))
    material->setTexture(TextureType::Albedo, albedo);
  else if (auto diffuse = loadMaterialTexture(source, aiTextureType_DIFFUSE,
                                              directory, true))
    material->setTexture(TextureType::Albedo, diffuse);

  // glTF and FBX use NORMALS. The OBJ importer maps map_Bump/bump to HEIGHT,
  // which some files use for real normal maps and others for grayscale bump
  // maps: only multi-channel images are treated as normal maps.
  auto normal =
      loadMaterialTexture(source, aiTextureType_NORMALS, directory, false);
  if (!normal) {
    normal =
        loadMaterialTexture(source, aiTextureType_HEIGHT, directory, false);
    if (normal && normal->getChannels() < 3)
      normal.reset();
  }
  if (normal)
    material->setTexture(TextureType::Normal, normal);

  // Assimp only sets the metallic factor for metallic-roughness formats
  // (glTF, some FBX). Everything else is described with Phong parameters.
  float metallic = 0.0f;
  if (source.Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
    importMetallicRoughness(source, directory, *material);
  else
    convertPhongParameters(source, directory, *material);
  return material;
}

void ResourceManager::importMetallicRoughness(const aiMaterial &source,
                                              const std::string &directory,
                                              Material &material) {
  // Factors always multiply the maps in glTF
  aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f);
  if (source.Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
    material.setAlbedo(
        glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a));

  float factor = 1.0f;
  if (source.Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS)
    material.setMetallic(factor);
  factor = 1.0f;
  if (source.Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == AI_SUCCESS)
    material.setRoughness(factor);

  // Assimp exposes the packed glTF texture (roughness G, metallic B) under
  // both the metalness and roughness types
  if (auto metallicRoughness = loadMaterialTexture(
          source, aiTextureType_METALNESS, directory, false))
    material.setTexture(TextureType::MetallicRoughness, metallicRoughness);

  // glTF occlusion arrives as a lightmap, other formats as ambient occlusion
  auto occlusion =
      loadMaterialTexture(source, aiTextureType_LIGHTMAP, directory, false);
  if (!occlusion)
    occlusion = loadMaterialTexture(source, aiTextureType_AMBIENT_OCCLUSION,
                                    directory, false);
  if (occlusion)
    material.setTexture(TextureType::Occlusion, occlusion);

  importEmissive(source, directory, material);

  aiString alphaMode;
  if (source.Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
    const std::string mode = alphaMode.C_Str();
    float cutoff = 0.5f;
    source.Get(AI_MATKEY_GLTF_ALPHACUTOFF, cutoff);
    if (mode == "OPAQUE")
      material.setAlphaMode(AlphaMode::Opaque);
    else
      // Blending is not supported yet, BLEND is approximated with a cut-out
      material.setAlphaMode(AlphaMode::Mask, cutoff);
  }
}

void ResourceManager::importEmissive(const aiMaterial &source,
                                     const std::string &directory,
                                     Material &material) {
  aiColor3D emissive(0.0f, 0.0f, 0.0f);
  source.Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
  float strength = 1.0f;
  source.Get(AI_MATKEY_EMISSIVE_INTENSITY, strength);

  if (auto map = loadMaterialTexture(source, aiTextureType_EMISSIVE, directory,
                                     true)) {
    material.setTexture(TextureType::Emissive, map);
    // A map with a black factor would be invisible, some exporters omit it
    if (emissive.IsBlack())
      emissive = aiColor3D(1.0f, 1.0f, 1.0f);
  }
  material.setEmissive(glm::vec3(emissive.r, emissive.g, emissive.b) *
                       strength);
}

void ResourceManager::convertPhongParameters(const aiMaterial &source,
                                             const std::string &directory,
                                             Material &material) {
  // Color factors only apply when there is no map, as OBJ exporters write
  // arbitrary Kd values next to map_Kd.
  aiColor3D color;
  if (!material.hasTexture(TextureType::Albedo) &&
      source.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
    material.setAlbedo(glm::vec3(color.r, color.g, color.b));

  // A Phong specular map or color becomes the reflectance of the dielectric
  if (auto specular = loadMaterialTexture(source, aiTextureType_SPECULAR,
                                          directory, false)) {
    material.setTexture(TextureType::Specular, specular);
    material.setSpecular(1.0f);
  } else if (source.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
    material.setSpecular(std::max({color.r, color.g, color.b}));
  }

  // Roughness giving about the same highlight size as the Blinn-Phong
  // exponent (Walter et al. 2007). Clamped because exporters often write very
  // high exponents that would turn everything into a mirror.
  float shininess = 0.0f;
  if (source.Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS &&
      shininess > 0.0f)
    material.setRoughness(
        std::clamp(std::sqrt(2.0f / (shininess + 2.0f)), 0.2f, 1.0f));

  // Ke and map_Ke
  importEmissive(source, directory, material);
}

std::shared_ptr<Model>
ResourceManager::loadModel(const std::string &path,
                           const ModelImportOptions &options) {
  const std::string key = path + (options.flipUVs ? "|flipUVs" : "");
  if (const auto it = sModelCache.find(key); it != sModelCache.end())
    return it->second;

  auto model = ModelLoader::load(path, options);
  sModelCache[key] = model;
  return model;
}

std::vector<Entity>
ResourceManager::instantiateModel(Scene &scene, const std::string &path,
                                  const Transform &transform,
                                  const ModelImportOptions &options) {
  const auto model = loadModel(path, options);

  std::vector<Entity> entities;
  entities.reserve(model->subMeshes.size());
  for (const auto &[name, mesh, material] : model->subMeshes) {
    Entity entity = scene.createEntity(name);
    entity.addComponent<Transform>(transform);
    entity.addComponent<MeshRenderer>(mesh, material);
    entities.push_back(entity);
  }
  return entities;
}

void ResourceManager::clearCache() {
  sTextureCache.clear();
  sShaderCache.clear();
  sModelCache.clear();
}
