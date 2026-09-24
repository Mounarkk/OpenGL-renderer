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

  /// Turns the sun around the vertical axis.
  void rotateSun(float angleRadians);

  /// Places the sun, angles in degrees (see getSunAzimuth/getSunElevation).
  void setSun(float azimuthDegrees, float elevationDegrees);

  /// Raises or lowers the sun, kept between the horizon and the zenith.
  void tiltSun(float angleRadians);

  /// Compass angle of the sun in degrees, 0 towards +X, 90 towards +Z.
  [[nodiscard]] float getSunAzimuth() const;

  /// Height of the sun above the horizon in degrees.
  [[nodiscard]] float getSunElevation() const;

  /// Enables the point and spot lights. With them off, only the sun lights
  /// the scene, which makes its shadows easier to read.
  void setLocalLightsEnabled(bool enabled) { mLocalLightsEnabled = enabled; }
  [[nodiscard]] bool areLocalLightsEnabled() const {
    return mLocalLightsEnabled;
  }

  /// Uploads every light to the `uDirLight`, `uPointLights` and `uSpotLight`
  /// uniforms.
  void bindLights(const Shader &shader) const;

  LightManager(const LightManager &) = delete;
  LightManager &operator=(const LightManager &) = delete;

private:
  LightManager();

  /// Rebuilds the directional light direction from the two angles.
  void updateSunDirection();

  LightData mLights;
  float mSunAzimuth = 0.0f;   ///< Radians
  float mSunElevation = 0.0f; ///< Radians
  bool mLocalLightsEnabled = true;
};
