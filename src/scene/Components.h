#pragma once
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include <memory>
#include <string>

// Forward declarations
class Mesh;
class Material;

/**
 * Tag component for entity identification and debugging.
 * Every entity should have a Tag component with a descriptive name.
 */
struct Tag {
  std::string name = "Entity";  // Human-readable name for the entity
};

/**
 * Transform component for 3D position, rotation, and scale.
 * 
 * This component represents an entity's transformation in 3D space using
 * separate position, rotation (Euler angles), and scale vectors. The rotation
 * uses Euler angles in degrees for simplicity, though this can lead to gimbal
 * lock in extreme cases. The component provides a helper method to generate
 * the final transformation matrix for rendering.
 * 
 * Rotation order: X (pitch) -> Y (yaw) -> Z (roll)
 * Coordinate system: Right-handed, Y-up
 */
struct Transform {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};  // World position
  glm::vec3 rotation = {0.0f, 0.0f, 0.0f};  // Euler angles in degrees (X=pitch, Y=yaw, Z=roll)
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};     // Scale factors per axis

  /**
   * Calculates the world transformation matrix from position, rotation, and scale.
   * Applies transformations in the order: Scale -> Rotate -> Translate
   * @return 4x4 transformation matrix for rendering
   */
  [[nodiscard]] glm::mat4 getWorldMatrix() const {
    glm::mat4 transform(1.0f);  // Initialize identity matrix
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, rotation.z, glm::vec3(0, 0, 1));  // Roll
    transform = glm::rotate(transform, rotation.y, glm::vec3(0, 1, 0));  // Yaw
    transform = glm::rotate(transform, rotation.x, glm::vec3(1, 0, 0));  // Pitch
    transform = glm::scale(transform, scale);
    return transform;
  }
};

/**
 * MeshRenderer component for entities that should be rendered.
 * 
 * This component associates a mesh (geometry) and material (surface properties)
 * with an entity, making it visible in the rendered scene. The mesh defines
 * the shape and the material defines how light interacts with the surface.
 * Both are shared resources that can be used by multiple entities.
 */
struct MeshRenderer {
  std::shared_ptr<Mesh> mesh;        // Geometry to render
  std::shared_ptr<Material> material; // Surface properties and textures
};
