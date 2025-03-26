#pragma once
#include "../gl/Shader.h"
#include "../scene/Components.h"
#include "../scene/Scene.h"
#include "Renderer.h"

// TODO: All values are initialized to default ones, have to make a system to
// that they can be changed
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
  DirectionalLight directionalLights;
  std::vector<PointLight> pointLights;
  std::vector<SpotLight> spotLights;
};

class LightManager {
public:
  [[nodiscard]] LightData getLights();
  static void bindLights(Shader& shader);

  static LightManager* getInstance();
private:
  LightManager();

  LightData mLights;
  // Static pointer to the instance
  static LightManager* pInstancePtr;
  // Mutex to ensure thread safety
  static std::mutex mMtx;
};

