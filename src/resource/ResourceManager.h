#pragma once
#include "../gl/Texture.h"
#include "../gl/Shader.h"
#include "../rendering/Material.h"
#include "assimp/material.h"
#include <memory>
#include <unordered_map>

class ResourceManager {
public:
  static std::shared_ptr<Texture> loadTexture(const std::string& path, bool sRGB = false);
  static std::shared_ptr<Shader> loadShader(const std::string& vsPath, const std::string& fsPath);
  static std::shared_ptr<Material> ResourceManager::loadMaterial(aiMaterial* aiMaterial);
  static std::vector<std::shared_ptr<Texture>> ResourceManager::loadMaterialTextures(aiMaterial *mat, aiTextureType type);
private:
  static std::unordered_map<std::string, std::weak_ptr<Texture>> sTextureCache;
  static std::unordered_map<std::string, std::weak_ptr<Shader>> sShaderCache;
};
