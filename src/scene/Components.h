#pragma once
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include <memory>
#include <string>

// Forward declarations
class Mesh;
class Material;

struct Tag {
  std::string name = "Entity";
};

struct Transform {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};

  [[nodiscard]] glm::mat4 getWorldMatrix() const {
    glm::mat4 transform(1.0f);  // Initialize identity matrix
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, rotation.z, glm::vec3(0, 0, 1));
    transform = glm::rotate(transform, rotation.y, glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, rotation.x, glm::vec3(1, 0, 0));
    transform = glm::scale(transform, scale);
    return transform;
  }
};

struct MeshRenderer {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};
