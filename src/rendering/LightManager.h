#pragma once
#include "../gl/Shader.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <array>

/// Sun-like light. `direction` points from the light towards the scene.
struct DirectionalLight {
  glm::vec3 direction = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));

  glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
  glm::vec3 diffuse = {0.8f, 0.8f, 0.8f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};
};

/// Omni light with constant/linear/quadratic attenuation.
struct PointLight {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};

  glm::vec3 ambient = {0.02f, 0.02f, 0.02f};
  glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};

  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
};

/// Cone light. Cut-offs are stored as cosines of the half angles.
struct SpotLight {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 direction = {0.0f, 0.0f, -1.0f};

  glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
  glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
  glm::vec3 specular = {1.0f, 1.0f, 1.0f};

  float cutOff = glm::cos(glm::radians(12.5f));
  float outerCutOff = glm::cos(glm::radians(17.5f));
};

/// Must match NB_POINT_LIGHTS in forward_shader.frag.
constexpr int kPointLightCount = 4;

struct LightData {
  DirectionalLight directionalLight;
  std::array<PointLight, kPointLightCount> pointLights;
  SpotLight spotLight;
};

/**
 * Owns the lights of the scene and uploads them to the lighting shader.
 *
 * Only the directional light casts shadows for now.
 */
class LightManager {
public:
  static LightManager &getInstance();

  [[nodiscard]] const LightData &getLights() const { return mLights; }
  [[nodiscard]] LightData &getLights() { return mLights; }

  /// Rotates the directional light around the world Y axis.
  void rotateSun(float angleRadians);

  /// Uploads every light to the `uDirLight`, `uPointLights` and `uSpotLight`
  /// uniforms.
  void bindLights(const Shader &shader) const;

  LightManager(const LightManager &) = delete;
  LightManager &operator=(const LightManager &) = delete;

private:
  LightManager();

  LightData mLights;
};
