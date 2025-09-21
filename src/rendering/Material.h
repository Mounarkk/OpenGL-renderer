#pragma once
#include "../gl/Shader.h"
#include "../gl/Texture.h"
#include <memory>
#include <unordered_map>

/**
 * Types of textures that can be used in PBR (Physically Based Rendering) materials.
 * Each type serves a specific purpose in the rendering equation.
 */
enum class TextureType {
  Albedo,            // Base color/diffuse texture
  Specular,          // Specular reflection texture 
  Normal,            // Normal map for surface detail
  Metallic,          // Metallic/non-metallic mask
  Roughness,         // Surface roughness texture
  AmbientOcclusion,  // Ambient occlusion for shadowing
  Emissive           // Self-illumination texture
};

/**
 * Material system for managing surface properties and textures.
 * 
 * The Material class encapsulates all surface properties needed for realistic
 * rendering, including both PBR (Physically Based Rendering) parameters and
 * texture maps. It provides a unified interface for setting material properties
 * and automatically binds textures and uniforms to shaders during rendering.
 * 
 * Key features:
 * - PBR material parameters (albedo, metallic, roughness)
 * - Multi-texture support for detailed surface representation
 * - Automatic shader uniform binding
 * - Flexible texture type system for different rendering techniques
 * - Memory-efficient shared texture resources
 */
class Material {
public:
  /**
   * Default constructor that initializes material with default PBR values.
   */
  Material() = default;
  
  /**
   * Destructor that cleans up material resources.
   */
  ~Material();
  
  /**
   * Manually cleans up material resources.
   * Called automatically by destructor.
   */
  void clean();

  /**
   * Binds all material properties and textures to the specified shader.
   * Sets uniform variables and binds textures to appropriate texture units.
   * @param shader Shader program to bind material properties to
   */
  void bind(const std::shared_ptr<Shader> &shader) const;

  // PBR (Physically Based Rendering) parameter setters
  
  /**
   * Sets the base color (albedo) of the material.
   * @param albedo RGB color values in linear space (0.0 to 1.0)
   */
  void setAlbedo(const glm::vec3 &albedo);
  
  /**
   * Sets the metallic factor of the material.
   * @param metallic Metallic value (0.0 = dielectric, 1.0 = metallic)
   */
  void setMetallic(float metallic);
  
  /**
   * Sets the roughness factor of the material.
   * @param roughness Roughness value (0.0 = mirror smooth, 1.0 = completely rough)
   */
  void setRoughness(float roughness);

  /**
   * Associates a texture with this material for the specified type.
   * @param type Type of texture (albedo, normal, metallic, etc.)
   * @param texture Shared pointer to the texture resource
   */
  void setTexture(TextureType type, const std::shared_ptr<Texture> &texture);
private:
  glm::vec3 mAlbedo = {1.0f, 1.0f, 1.0f}; // Base color
  float mMetallic = 0.0f;  // Metallic factor (0 = dielectric, 1 = metal)
  float mRoughness = 0.5f; // Roughness factor (0 = smooth, 1 = rough)
  float mShininess = 64.f;

  std::unordered_map<TextureType, std::shared_ptr<Texture>> m_Textures;
};
