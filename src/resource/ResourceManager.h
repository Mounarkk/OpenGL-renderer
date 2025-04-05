#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include "../rendering/Material.h"
#include "assimp/material.h"
#include <memory>
#include <unordered_map>

class ResourceManager {
public:
  static std::shared_ptr<Texture> loadTexture(const std::string &path,
                                              bool sRGB = false);
  static std::shared_ptr<Shader> loadShader(const std::string &vsPath,
                                            const std::string &fsPath);
  static std::shared_ptr<Material>
  loadMaterial(const aiMaterial *aiMaterial);
  static std::vector<std::shared_ptr<Texture>>
  loadMaterialTextures(const aiMaterial *mat, aiTextureType type);

private:
  static std::unordered_map<std::string, std::weak_ptr<Texture>> sTextureCache;
  static std::unordered_map<std::string, std::weak_ptr<Shader>> sShaderCache;
};
