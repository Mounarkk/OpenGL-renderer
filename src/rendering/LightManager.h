#pragma once
#include "../gl/Shader.h"
#include "../scene/Components.h"

/**
 * Directional light structure with default lighting values.
 * These values provide good general-purpose lighting for most scenes.
 */
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

struct LightData {
  DirectionalLight directionalLight;
  std::vector<PointLight> pointLights;
  SpotLight spotLight;
};

class LightManager {
public:
  [[nodiscard]] LightData getLights();
  void bindLights(const std::shared_ptr<Shader> &shader) const;

  static LightManager &getInstance();

private:
  LightManager();
  ~LightManager() = default;

  // No copying or assignment
  LightManager(const LightManager &) = delete;
  LightManager &operator=(const LightManager &) = delete;

  LightData mLights;
};
