#include "ResourceManager.h"

std::unordered_map<std::string, std::weak_ptr<Texture>> ResourceManager::sTextureCache;
std::unordered_map<std::string, std::weak_ptr<Shader>> ResourceManager::sShaderCache;

std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& path, bool sRGB) {
  auto it = sTextureCache.find(path);
  if (it != sTextureCache.end()) {
    if (auto texture = it->second.lock())
      return texture;
  }

  auto texture = std::make_shared<Texture>(path, sRGB);
  sTextureCache[path] = texture;
  return texture;
}

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& vsPath, const std::string& fsPath) {
  std::string key = vsPath + "|" + fsPath;
  auto it = sShaderCache.find(key);
  if (it != sShaderCache.end()) {
    if (auto shader = it->second.lock())
      return shader;
  }

  auto shader = std::make_shared<Shader>(vsPath, fsPath);
  sShaderCache[key] = shader;
  return shader;
}
