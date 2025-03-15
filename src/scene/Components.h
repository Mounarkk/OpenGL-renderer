#pragma once
#include <ext/matrix_transform.hpp>
#include <fwd.hpp>
#include <memory>
#include <string>
#include <vec3.hpp>

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
    glm::mat4 transform = translate(transform, position);
    transform = rotate(transform, rotation.z, {0, 0, 1});
    transform = rotate(transform, rotation.y, {0, 1, 0});
    transform = rotate(transform, rotation.x, {1, 0, 0});
    transform = glm::scale(transform, scale);
    return transform;
  }
};

struct MeshRenderer {
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};