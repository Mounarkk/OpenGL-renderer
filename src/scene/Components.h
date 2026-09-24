#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

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
