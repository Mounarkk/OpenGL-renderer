#include "ResourceManager.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "../scene/Scene.h"
#include "ModelLoader.h"
#include <functional>

// Static member definitions
std::unordered_map<std::string, std::weak_ptr<Texture>>
    ResourceManager::sTextureCache;
std::unordered_map<std::string, std::weak_ptr<Shader>>
    ResourceManager::sShaderCache;
std::unordered_map<std::string, ResourceManager::ModelCacheEntry>
    ResourceManager::sModelCache;

std::shared_ptr<Material>
ResourceManager::loadMaterial(const aiMaterial *aiMaterial,
                              const std::string &path) {
  // Create a new material
  auto material = std::make_shared<Material>();

  // Load textures
  const auto diffuseTextures =
      loadMaterialTextures(aiMaterial, aiTextureType_DIFFUSE, path);
  const auto specularTextures =
      loadMaterialTextures(aiMaterial, aiTextureType_SPECULAR, path);
  const auto normalTextures =
      loadMaterialTextures(aiMaterial, aiTextureType_HEIGHT, path);

  // Assign textures to material
  if (!diffuseTextures.empty()) {
    material->setTexture(TextureType::Albedo, diffuseTextures[0]);
  }
  if (!specularTextures.empty()) {
    material->setTexture(TextureType::Specular, specularTextures[0]);
  }
  if (!normalTextures.empty()) {
    material->setTexture(TextureType::Normal, normalTextures[0]);
  }

  return material;
}

std::vector<std::shared_ptr<Texture>> ResourceManager::loadMaterialTextures(
    const aiMaterial *mat, const aiTextureType type, const std::string &path) {
  std::vector<std::shared_ptr<Texture>> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    textures.push_back(loadTexture(path.c_str() + std::string(str.C_Str())));
  }
  return textures;
}

std::shared_ptr<Texture> ResourceManager::loadTexture(const std::string &path,
                                                      bool sRGB) {
  if (const auto it = sTextureCache.find(path); it != sTextureCache.end()) {
    if (auto texture = it->second.lock()) {
      Logger::get()->debug("Using cached texture: {}", path);
      return texture;
    }
  }

  Logger::get()->debug("Loading new texture: {}", path);
  try {
    auto texture = std::make_shared<Texture>(path, sRGB);
    sTextureCache[path] = texture;
    return texture;
  } catch (const std::exception &e) {
    Logger::get()->error("Failed to load texture: {}", path);
    throw ResourceException(path, "Texture loading failed - " +
                                      std::string(e.what()));
  }
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string &vsPath,
                                                    const std::string &fsPath) {
  const std::string key = vsPath + "|" + fsPath;
  if (const auto it = sShaderCache.find(key); it != sShaderCache.end()) {
    if (auto shader = it->second.lock()) {
      Logger::get()->debug("Using cached shader: {} | {}", vsPath, fsPath);
      return shader;
    }
  }

  Logger::get()->debug("Loading new shader: {} | {}", vsPath, fsPath);
  try {
    auto shader = std::make_shared<Shader>(vsPath, fsPath);
    sShaderCache[key] = shader;
    return shader;
  } catch (const ShaderException &e) {
    Logger::get()->error("Failed to load shader: {} | {}", vsPath, fsPath);
    throw; // Re-throw the ShaderException as-is
  } catch (const std::exception &e) {
    Logger::get()->error("Failed to load shader: {} | {}", vsPath, fsPath);
    throw ResourceException(vsPath + " | " + fsPath,
                            "Shader loading failed - " + std::string(e.what()));
  }
}

Entity ResourceManager::loadModel(Scene &scene, const std::string &path, 
                                 const Transform* customTransform) {
  // Note: When using custom transforms, we skip caching to allow multiple instances
  // with different transforms. Only cache models with default transforms.
  if (!customTransform) {
    // Check if model is already loaded in cache
    if (const auto it = sModelCache.find(path); it != sModelCache.end()) {
      // Check if the cached scene is still valid and the entity exists
      if (auto cachedScene = it->second.scene.lock()) {
        if (cachedScene.get() == &scene &&
            it->second.rootEntityHandle != entt::null) {
          Entity cachedEntity(it->second.rootEntityHandle, &scene);
          if (cachedEntity) { // Use operator bool() to check validity
            Logger::get()->info("Using cached model: {}", path);
            return cachedEntity;
          }
        }
      }
      // Remove invalid cache entry
      sModelCache.erase(it);
    }
  }

  // Load the model using ModelLoader
  Logger::get()->info("Loading model: {}", path);
  std::string modelPath =
      path; // ModelLoader modifies the path, so we need a copy
  Entity rootEntity = ModelLoader::load(scene, modelPath, customTransform);

  if (rootEntity) { // Use operator bool() to check validity
    // Only cache models with default transforms to allow multiple instances
    if (!customTransform) {
      ModelCacheEntry entry(
          std::shared_ptr<Scene>(&scene, [](Scene *) {}), // Non-owning shared_ptr
          rootEntity.handle());
      sModelCache[path] = entry;
      Logger::get()->info("Successfully loaded and cached model: {}", path);
    } else {
      Logger::get()->info("Successfully loaded model with custom transform: {}", path);
    }
  } else {
    Logger::get()->error("Failed to load model: {}", path);
    throw ResourceException(
        path,
        "Model loading failed - invalid model data or unsupported format");
  }

  return rootEntity;
}

ResourceManager::CacheStats ResourceManager::getCacheStats() {
  CacheStats stats;

  // Count valid entries in texture cache
  stats.textureCount = 0;
  for (auto it = sTextureCache.begin(); it != sTextureCache.end();) {
    if (it->second.expired()) {
      it = sTextureCache.erase(it);
    } else {
      ++stats.textureCount;
      ++it;
    }
  }

  // Count valid entries in shader cache
  stats.shaderCount = 0;
  for (auto it = sShaderCache.begin(); it != sShaderCache.end();) {
    if (it->second.expired()) {
      it = sShaderCache.erase(it);
    } else {
      ++stats.shaderCount;
      ++it;
    }
  }

  // Count valid entries in model cache
  stats.modelCount = 0;
  for (auto it = sModelCache.begin(); it != sModelCache.end();) {
    if (it->second.scene.expired() ||
        it->second.rootEntityHandle == entt::null) {
      it = sModelCache.erase(it);
    } else {
      ++stats.modelCount;
      ++it;
    }
  }

  return stats;
}

void ResourceManager::clearCache() {
  Logger::get()->info("Clearing resource cache");
  sTextureCache.clear();
  sShaderCache.clear();
  sModelCache.clear();
}
