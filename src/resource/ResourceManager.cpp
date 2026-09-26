#include "ResourceManager.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "../rendering/ShaderInterface.h"
#include "../scene/Scene.h"
#include "ModelLoader.h"

#include <assimp/material.h>

#include <algorithm>

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

  if (auto specular =
          loadMaterialTexture(source, aiTextureType_SPECULAR, directory, false))
    material->setTexture(TextureType::Specular, specular);

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

  // Color factors only apply when there is no map, as OBJ exporters write
  // arbitrary Kd values next to map_Kd.
  aiColor3D color;
  if (!material->hasTexture(TextureType::Albedo) &&
      source.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
    material->setAlbedo({color.r, color.g, color.b});
  if (!material->hasTexture(TextureType::Specular) &&
      source.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
    material->setSpecular({color.r, color.g, color.b});

  float shininess = 0.0f;
  if (source.Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS &&
      shininess > 0.0f)
    material->setShininess(shininess);

  return material;
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
