#include "ResourceManager.h"

std::unordered_map<std::string, std::weak_ptr<Texture>>
    ResourceManager::sTextureCache;
std::unordered_map<std::string, std::weak_ptr<Shader>>
    ResourceManager::sShaderCache;

std::shared_ptr<Material>
ResourceManager::loadMaterial(const aiMaterial *aiMaterial, const std::string &path) {
  // Create a new material
  auto material = std::make_shared<Material>();

  // Load textures
  const auto diffuseTextures =
      loadMaterialTextures(aiMaterial, aiTextureType_DIFFUSE, path);
  const auto specularTextures =
      loadMaterialTextures(aiMaterial, aiTextureType_SPECULAR, path);

  // Assign textures to material
  if (!diffuseTextures.empty()) {
    material->setTexture(TextureType::Albedo, diffuseTextures[0]);
  }
  if (!specularTextures.empty()) {
    material->setTexture(TextureType::Specular, specularTextures[0]);
  }

  return material;
}

std::vector<std::shared_ptr<Texture>>
ResourceManager::loadMaterialTextures(const aiMaterial *mat,
                                      const aiTextureType type,
                                      const std::string &path) {
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
    if (auto texture = it->second.lock())
      return texture;
  }

  auto texture = std::make_shared<Texture>(path, sRGB);
  sTextureCache[path] = texture;
  return texture;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string &vsPath,
                                                    const std::string &fsPath) {
  const std::string key = vsPath + "|" + fsPath;
  if (const auto it = sShaderCache.find(key); it != sShaderCache.end()) {
    if (auto shader = it->second.lock())
      return shader;
  }

  auto shader = std::make_shared<Shader>(vsPath, fsPath);
  sShaderCache[key] = shader;
  return shader;
}
