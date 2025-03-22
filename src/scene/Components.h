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

// Light components
// TODO: All values are initialized to default ones, have to make a system to that they can be changed
struct DirectionalLight {
  glm::vec3 direction = {-0.2f, -1.0f, -0.3f};

  glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
  glm::vec3 diffuse = {0.8f, 0.8f, 0.8f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};
};

struct PointLight {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};

  glm::vec3 ambient = {0.02f, 0.02f, 0.02f};
  glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};

  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
};

struct SpotLight {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 direction = {0.0f, 0.0f, -1.0f};

  glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
  glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};

  float cutOff = glm::cos(glm::radians(12.5f));
  float outerCutOff = glm::cos(glm::radians(17.5f));
};
