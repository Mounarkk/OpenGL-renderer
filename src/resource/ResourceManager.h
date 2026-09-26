#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include "../rendering/Material.h"
#include "../scene/Components.h"
#include "../scene/Entity.h"
#include "Model.h"

#include <assimp/material.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Scene;

/**
 * Loads and caches textures, shaders and models.
 *
 * Textures and shaders are cached with weak pointers: they are shared while
 * something uses them and freed afterwards. Models are kept alive by the
 * cache until clearCache(), so instantiating the same file again is free.
 *
 * Not thread-safe, everything happens on the thread owning the GL context.
 */
class ResourceManager {
public:
  /// Throws a ResourceException if the image cannot be loaded.
  static std::shared_ptr<Texture> loadTexture(const std::string &path,
                                              bool sRGB);

  /// Compiles with the ShaderInterface defines. Throws a ShaderException on
  /// compilation or link errors.
  static std::shared_ptr<Shader> loadShader(const std::string &vsPath,
                                            const std::string &fsPath,
                                            const std::string &gsPath = "");

  /**
   * Converts an Assimp material. Missing or unreadable textures are logged
   * and replaced by the material defaults instead of failing the import.
   * @param directory Directory the texture paths are relative to
   */
  static std::shared_ptr<Material> loadMaterial(const aiMaterial &material,
                                                const std::string &directory);

  /// Imports a model file once and returns the cached result afterwards.
  static std::shared_ptr<Model>
  loadModel(const std::string &path, const ModelImportOptions &options = {});

  /**
   * Creates one entity per sub-mesh of the model, all sharing `transform`.
   * @return The created entities
   */
  static std::vector<Entity>
  instantiateModel(Scene &scene, const std::string &path,
                   const Transform &transform = {},
                   const ModelImportOptions &options = {});

  /// Drops every cache entry. Resources still in use stay alive.
  static void clearCache();

private:
  static std::unordered_map<std::string, std::weak_ptr<Texture>> sTextureCache;
  static std::unordered_map<std::string, std::weak_ptr<Shader>> sShaderCache;
  static std::unordered_map<std::string, std::shared_ptr<Model>> sModelCache;

  static std::shared_ptr<Texture>
  loadMaterialTexture(const aiMaterial &material, aiTextureType type,
                      const std::string &directory, bool sRGB);
};
