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
  std::string name = "Entity"; // Human-readable name for the entity
};

/**
 * Transform component for 3D position, rotation, and scale.
 *
 * This component represents an entity's transformation in 3D space using
 * separate position, rotation (Euler angles), and scale vectors. The rotation
 * uses Euler angles in radians for consistency with GLM, though this can lead
 * to gimbal lock in extreme cases. The component provides helper methods to
 * generate the final transformation matrix and perform common transform
 * operations.
 *
 * Coordinate System: Right-handed, Y-up (OpenGL standard)
 * - X-axis: Right
 * - Y-axis: Up
 * - Z-axis: Forward (towards viewer)
 *
 * Rotation Order: X (pitch) -> Y (yaw) -> Z (roll)
 * - X rotation (pitch): Rotation around X-axis (nodding up/down)
 * - Y rotation (yaw): Rotation around Y-axis (turning left/right)
 * - Z rotation (roll): Rotation around Z-axis (tilting left/right)
 *
 * Matrix Application Order: Scale -> Rotate -> Translate (SRT)
 */
struct Transform {
  glm::vec3 position = {0.0f, 0.0f, 0.0f}; // World position
  glm::vec3 rotation = {
      0.0f, 0.0f, 0.0f}; // Euler angles in radians (X=pitch, Y=yaw, Z=roll)
  glm::vec3 scale = {1.0f, 1.0f, 1.0f}; // Scale factors per axis

  /**
   * Calculates the world transformation matrix from position, rotation, and
   * scale. Applies transformations in the order: Scale -> Rotate -> Translate
   * (SRT)
   * @return 4x4 transformation matrix for rendering
   */
  [[nodiscard]] glm::mat4 getWorldMatrix() const { return getMatrix(); }

  /**
   * Calculates the transformation matrix from position, rotation, and scale.
   * This is the primary method for getting the transform matrix.
   * Applies transformations in the order: Scale -> Rotate -> Translate (SRT)
   * @return 4x4 transformation matrix
   */
  [[nodiscard]] glm::mat4 getMatrix() const {
    // Start with identity matrix
    glm::mat4 transform(1.0f);

    // Apply translation
    transform = glm::translate(transform, position);

    // Apply rotations in order: X (pitch) -> Y (yaw) -> Z (roll)
    transform = glm::rotate(transform, rotation.x, glm::vec3(1, 0, 0)); // Pitch
    transform = glm::rotate(transform, rotation.y, glm::vec3(0, 1, 0)); // Yaw
    transform = glm::rotate(transform, rotation.z, glm::vec3(0, 0, 1)); // Roll

    // Apply scale
    transform = glm::scale(transform, scale);

    return transform;
  }

  // ========== Utility Methods for Common Transform Operations ==========

  /**
   * Translates the transform by the given offset.
   * @param offset The translation offset to apply
   */
  void translate(const glm::vec3 &offset) { position += offset; }

  /**
   * Translates the transform by individual components.
   * @param x X-axis translation
   * @param y Y-axis translation
   * @param z Z-axis translation
   */
  void translate(float x, float y, float z) { position += glm::vec3(x, y, z); }

  /**
   * Rotates the transform by the given Euler angles (in radians).
   * @param eulerAngles Rotation angles in radians (X=pitch, Y=yaw, Z=roll)
   */
  void rotate(const glm::vec3 &eulerAngles) { rotation += eulerAngles; }

  /**
   * Rotates the transform by individual Euler angles (in radians).
   * @param pitch Rotation around X-axis (radians)
   * @param yaw Rotation around Y-axis (radians)
   * @param roll Rotation around Z-axis (radians)
   */
  void rotate(float pitch, float yaw, float roll) {
    rotation += glm::vec3(pitch, yaw, roll);
  }

  /**
   * Rotates the transform around a specific axis by the given angle.
   * @param angle Rotation angle in radians
   * @param axis Normalized axis of rotation
   */
  void rotateAround(float angle, const glm::vec3 &axis) {
    // For single-axis rotations, map to appropriate Euler angle
    if (glm::abs(glm::dot(axis, glm::vec3(1, 0, 0))) > 0.99f) {
      rotation.x += angle; // X-axis rotation
    } else if (glm::abs(glm::dot(axis, glm::vec3(0, 1, 0))) > 0.99f) {
      rotation.y += angle; // Y-axis rotation
    } else if (glm::abs(glm::dot(axis, glm::vec3(0, 0, 1))) > 0.99f) {
      rotation.z += angle; // Z-axis rotation
    }
    // Note: For arbitrary axes, this is a simplified approach.
    // A full implementation would require quaternion conversion.
  }

  /**
   * Scales the transform by the given factors.
   * @param scaleFactors Scale factors for each axis
   */
  void scaleBy(const glm::vec3 &scaleFactors) { scale *= scaleFactors; }

  /**
   * Scales the transform uniformly by the given factor.
   * @param scaleFactor Uniform scale factor for all axes
   */
  void scaleBy(float scaleFactor) { scale *= scaleFactor; }

  /**
   * Scales the transform by individual factors.
   * @param x X-axis scale factor
   * @param y Y-axis scale factor
   * @param z Z-axis scale factor
   */
  void scaleBy(float x, float y, float z) { scale *= glm::vec3(x, y, z); }

  // ========== Convenience Methods ==========

  /**
   * Sets the position to the given coordinates.
   * @param newPosition New world position
   */
  void setPosition(const glm::vec3 &newPosition) { position = newPosition; }

  /**
   * Sets the position to the given coordinates.
   * @param x X coordinate
   * @param y Y coordinate
   * @param z Z coordinate
   */
  void setPosition(float x, float y, float z) { position = glm::vec3(x, y, z); }

  /**
   * Sets the rotation to the given Euler angles (in radians).
   * @param eulerAngles New rotation angles in radians
   */
  void setRotation(const glm::vec3 &eulerAngles) { rotation = eulerAngles; }

  /**
   * Sets the rotation to the given Euler angles (in radians).
   * @param pitch Rotation around X-axis (radians)
   * @param yaw Rotation around Y-axis (radians)
   * @param roll Rotation around Z-axis (radians)
   */
  void setRotation(float pitch, float yaw, float roll) {
    rotation = glm::vec3(pitch, yaw, roll);
  }

  /**
   * Sets the scale to the given factors.
   * @param newScale New scale factors
   */
  void setScale(const glm::vec3 &newScale) { scale = newScale; }

  /**
   * Sets the scale to the given uniform factor.
   * @param uniformScale Uniform scale factor for all axes
   */
  void setScale(float uniformScale) { scale = glm::vec3(uniformScale); }

  /**
   * Sets the scale to the given factors.
   * @param x X-axis scale factor
   * @param y Y-axis scale factor
   * @param z Z-axis scale factor
   */
  void setScale(float x, float y, float z) { scale = glm::vec3(x, y, z); }

  // ========== Helper Methods ==========

  /**
   * Resets the transform to identity (position=0, rotation=0, scale=1).
   */
  void reset() {
    position = glm::vec3(0.0f);
    rotation = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
  }

  /**
   * Gets the forward direction vector based on current rotation.
   * In right-handed Y-up coordinate system, forward is -Z direction.
   * @return Normalized forward direction vector
   */
  [[nodiscard]] glm::vec3 getForward() const {
    glm::mat4 rotMatrix =
        glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.y, glm::vec3(0, 1, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.z, glm::vec3(0, 0, 1));
    return glm::normalize(glm::vec3(rotMatrix * glm::vec4(0, 0, -1, 0)));
  }

  /**
   * Gets the right direction vector based on current rotation.
   * @return Normalized right direction vector
   */
  [[nodiscard]] glm::vec3 getRight() const {
    glm::mat4 rotMatrix =
        glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.y, glm::vec3(0, 1, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.z, glm::vec3(0, 0, 1));
    return glm::normalize(glm::vec3(rotMatrix * glm::vec4(1, 0, 0, 0)));
  }

  /**
   * Gets the up direction vector based on current rotation.
   * @return Normalized up direction vector
   */
  [[nodiscard]] glm::vec3 getUp() const {
    glm::mat4 rotMatrix =
        glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.y, glm::vec3(0, 1, 0));
    rotMatrix = glm::rotate(rotMatrix, rotation.z, glm::vec3(0, 0, 1));
    return glm::normalize(glm::vec3(rotMatrix * glm::vec4(0, 1, 0, 0)));
  }
};

/**
 * MeshRenderer component for entities that should be rendered.
 *
 * This component associates a mesh (geometry) and material (surface properties)
 * with an entity, making it visible in the rendered scene. The mesh defines
 * the shape and the material defines how light interacts with the surface.
 * Both are shared resources that can be used by multiple entities.
 *
 * Ownership Patterns:
 * - Uses std::shared_ptr for mesh and material (shared ownership)
 * - Multiple entities can safely share the same mesh/material
 * - Resources are automatically cleaned up when no entities reference them
 * - Component can be copied safely (shared_ptr handles reference counting)
 */
struct MeshRenderer {
  std::shared_ptr<Mesh> mesh;         // Geometry to render
  std::shared_ptr<Material> material; // Surface properties and textures
};
