#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <cmath>

#include <memory>
#include <string>

class Mesh;
class Material;

/// Human readable name, added to every entity by Scene::createEntity.
struct Tag {
  std::string name = "Entity";
};

/**
 * Position, orientation and scale of an entity.
 *
 * Rotation is stored as Euler angles in radians (x = pitch, y = yaw,
 * z = roll). The world matrix is T * Rx * Ry * Rz * S.
 */
struct Transform {
  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f};
  glm::vec3 scale{1.0f};

  [[nodiscard]] glm::mat4 getWorldMatrix() const {
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
    matrix = glm::rotate(matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    matrix = glm::rotate(matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    matrix = glm::rotate(matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::scale(matrix, scale);
  }

  /// Local -Z axis in world space.
  [[nodiscard]] glm::vec3 getForward() const {
    return glm::normalize(glm::vec3(rotationMatrix() * glm::vec4(0, 0, -1, 0)));
  }

  /// Local +X axis in world space.
  [[nodiscard]] glm::vec3 getRight() const {
    return glm::normalize(glm::vec3(rotationMatrix() * glm::vec4(1, 0, 0, 0)));
  }

  /// Local +Y axis in world space.
  [[nodiscard]] glm::vec3 getUp() const {
    return glm::normalize(glm::vec3(rotationMatrix() * glm::vec4(0, 1, 0, 0)));
  }

private:
  [[nodiscard]] glm::mat4 rotationMatrix() const {
    glm::mat4 matrix =
        glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    matrix = glm::rotate(matrix, rotation.y, glm::vec3(0, 1, 0));
    return glm::rotate(matrix, rotation.z, glm::vec3(0, 0, 1));
  }
};

/// Makes an entity visible: geometry plus the material to draw it with.
/// Both are shared, many entities can point to the same mesh and material.
struct MeshRenderer {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

/**
 * Light coming from infinitely far away in one direction, like the sun.
 * Only the first one found in the scene is used, and it casts the cascaded
 * shadows. Its direction comes from two angles rather than from a Transform
 * so that it can be driven like a sun in the sky.
 */
struct DirectionalLight {
  float azimuth = 56.0f;   ///< Degrees, 0 towards +X, 90 towards +Z
  float elevation = 70.0f; ///< Degrees above the horizon
  glm::vec3 color{1.0f};
  float intensity = 0.8f;
  glm::vec3 ambient{0.05f}; ///< Ambient light of the whole scene

  /// Direction the light travels in, from the sky towards the ground.
  [[nodiscard]] glm::vec3 getDirection() const {
    const float a = glm::radians(azimuth);
    const float e = glm::radians(elevation);
    return -glm::vec3(std::cos(e) * std::cos(a), std::sin(e),
                      std::cos(e) * std::sin(a));
  }
};

/// Omni light at the entity position. Its influence fades smoothly to zero at
/// `range`, so it can be culled beyond it.
struct PointLight {
  glm::vec3 color{1.0f};
  float intensity = 4.0f;
  float range = 8.0f;
};

/// Cone light at the entity position, shining along its local -Z axis
/// (Transform::getForward).
struct SpotLight {
  glm::vec3 color{1.0f};
  float intensity = 8.0f;
  float range = 15.0f;
  float innerAngle = 12.5f; ///< Degrees, full intensity inside
  float outerAngle = 17.5f; ///< Degrees, no light outside
};
