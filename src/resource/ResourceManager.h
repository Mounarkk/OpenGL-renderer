#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include "../rendering/Material.h"
#include "../scene/Entity.h"
#include "assimp/material.h"
#include <entt/entt.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

// Forward declarations
class Scene;

/**
 * Centralized resource manager for loading and caching assets.
 *
 * The ResourceManager provides a unified interface for loading and caching
 * various types of assets including textures, shaders, materials, and models.
 * It uses weak_ptr caching to automatically clean up unused resources while
 * avoiding duplicate loading of the same assets.
 *
 * Key features:
 * - Automatic caching with weak_ptr for memory efficiency
 * - Unified interface for all asset types
 * - Thread-safe resource loading (static methods with internal synchronization)
 * - Automatic cleanup of unused resources
 *
 * Ownership Patterns:
 * - Returns std::shared_ptr for all resources (shared ownership)
 * - Uses std::weak_ptr for caching to avoid circular references
 * - Does NOT own Assimp pointers (managed by Assimp library)
 * - Callers share ownership of returned resources
 * - Resources are automatically cleaned up when last shared_ptr is destroyed
 */
class ResourceManager {
public:
  /**
   * Loads a texture from file with caching support.
   * @param path File path to the texture
   * @param sRGB Whether to load as sRGB texture (default: false)
   * @return Shared pointer to the loaded texture
   */
  static std::shared_ptr<Texture> loadTexture(const std::string &path,
                                              bool sRGB = false);

  /**
   * Loads a shader from vertex and fragment shader files with caching.
   * @param vsPath Path to vertex shader file
   * @param fsPath Path to fragment shader file
   * @return Shared pointer to the compiled shader program
   */
  static std::shared_ptr<Shader> loadShader(const std::string &vsPath,
                                            const std::string &fsPath);

  /**
   * Creates a material from Assimp material data.
   * @param aiMaterial Assimp material to convert
   * @param path Base path for resolving texture paths
   * @return Shared pointer to the created material
   */
  static std::shared_ptr<Material> loadMaterial(const aiMaterial *aiMaterial,
                                                const std::string &path);

  /**
   * Loads textures for a specific material type from Assimp material.
   * @param mat Assimp material containing texture references
   * @param type Type of textures to load (diffuse, specular, etc.)
   * @param path Base path for resolving texture paths
   * @return Vector of loaded textures
   */
  static std::vector<std::shared_ptr<Texture>>
  loadMaterialTextures(const aiMaterial *mat, aiTextureType type,
                       const std::string &path);

  /**
   * Loads a 3D model from file and creates entities in the specified scene.
   * Uses caching to avoid reloading the same model multiple times.
   * @param scene Scene to create the model entities in
   * @param path File path to the 3D model (supports OBJ, FBX, GLTF, etc.)
   * @param customTransform Optional custom transform to apply to the root entity
   * @return Root entity of the loaded model hierarchy
   */
  static Entity loadModel(Scene &scene, const std::string &path, 
                         const Transform* customTransform = nullptr);

  /**
   * Gets the number of cached resources for debugging/monitoring.
   * @return Struct containing cache sizes for each resource type
   */
  struct CacheStats {
    size_t textureCount;
    size_t shaderCount;
    size_t modelCount;
  };
  static CacheStats getCacheStats();

  /**
   * Clears all cached resources. Useful for memory cleanup or hot-reloading.
   * Note: This only clears the cache entries, actual resources are cleaned up
   * when their last shared_ptr reference is released.
   */
  static void clearCache();

private:
  // Cache maps using weak_ptr for automatic cleanup
  static std::unordered_map<std::string, std::weak_ptr<Texture>> sTextureCache;
  static std::unordered_map<std::string, std::weak_ptr<Shader>> sShaderCache;

  // Model cache stores the root entity ID and scene reference
  // We use a simple struct to track loaded models
  struct ModelCacheEntry {
    std::weak_ptr<Scene> scene;
    entt::entity rootEntityHandle;

    ModelCacheEntry() : rootEntityHandle(entt::null) {}
    ModelCacheEntry(std::weak_ptr<Scene> s, entt::entity handle)
        : scene(s), rootEntityHandle(handle) {}
  };
  static std::unordered_map<std::string, ModelCacheEntry> sModelCache;

  /**
   * Helper template function to get or load a resource with caching.
   * @param key Resource path/key
   * @param cache Reference to the appropriate cache map
   * @param loader Function to load the resource if not cached
   * @return Shared pointer to the resource
   */
  template <typename T>
  static std::shared_ptr<T>
  getOrLoad(const std::string &key,
            std::unordered_map<std::string, std::weak_ptr<T>> &cache,
            std::function<std::shared_ptr<T>()> loader) {
    // Check if resource is already cached
    if (const auto it = cache.find(key); it != cache.end()) {
      if (auto resource = it->second.lock()) {
        return resource;
      }
      // Remove expired entry
      cache.erase(it);
    }

    // Load new resource
    auto resource = loader();
    if (resource) {
      cache[key] = resource;
    }
    return resource;
  }
};
