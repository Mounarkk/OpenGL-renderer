#include "LightManager.h"

#include <gtc/constants.hpp>

#include <cmath>
#include <string>

LightManager &LightManager::getInstance() {
  static LightManager instance;
  return instance;
}

LightManager::LightManager() {
  constexpr glm::vec3 positions[kPointLightCount] = {{0.7f, 0.2f, 2.0f},
                                                     {2.3f, -3.3f, -4.0f},
                                                     {-4.0f, 2.0f, -12.0f},
                                                     {0.0f, 0.0f, -3.0f}};

  for (int i = 0; i < kPointLightCount; ++i)
    mLights.pointLights[i].position = positions[i];

  // Angles of the default direction, as seen from the ground
  const glm::vec3 toSun = -mLights.directionalLight.direction;
  mSunAzimuth = std::atan2(toSun.z, toSun.x);
  mSunElevation = std::asin(toSun.y);
}

void LightManager::rotateSun(const float angleRadians) {
  mSunAzimuth = std::fmod(mSunAzimuth + angleRadians, glm::two_pi<float>());
  updateSunDirection();
}

void LightManager::setSun(const float azimuthDegrees,
                          const float elevationDegrees) {
  mSunAzimuth = glm::radians(azimuthDegrees);
  mSunElevation = 0.0f;
  tiltSun(glm::radians(elevationDegrees)); // applies the clamping
}

void LightManager::tiltSun(const float angleRadians) {
  // Below a couple of degrees the cascades get extremely long shadows, and
  // at the zenith the light view basis becomes degenerate.
  mSunElevation = glm::clamp(mSunElevation + angleRadians, glm::radians(2.0f),
                             glm::radians(89.0f));
  updateSunDirection();
}

float LightManager::getSunAzimuth() const {
  const float degrees = glm::degrees(mSunAzimuth);
  return degrees < 0.0f ? degrees + 360.0f : degrees;
}

float LightManager::getSunElevation() const {
  return glm::degrees(mSunElevation);
}

void LightManager::updateSunDirection() {
  const glm::vec3 toSun(std::cos(mSunElevation) * std::cos(mSunAzimuth),
                        std::sin(mSunElevation),
                        std::cos(mSunElevation) * std::sin(mSunAzimuth));
  mLights.directionalLight.direction = -glm::normalize(toSun);
}

void LightManager::bindLights(const Shader &shader) const {
  const auto &sun = mLights.directionalLight;
  shader.setVec3("uDirLight.direction", sun.direction);
  shader.setVec3("uDirLight.ambient", sun.ambient);
  shader.setVec3("uDirLight.diffuse", sun.diffuse);
  shader.setVec3("uDirLight.specular", sun.specular);

  // Disabled lights are uploaded black rather than removed, the shader
  // always loops over the same number of lights
  const float localScale = mLocalLightsEnabled ? 1.0f : 0.0f;

  for (int i = 0; i < kPointLightCount; ++i) {
    const auto &light = mLights.pointLights[i];
    const std::string name = "uPointLights[" + std::to_string(i) + "]";
    shader.setVec3(name + ".position", light.position);
    shader.setVec3(name + ".ambient", light.ambient * localScale);
    shader.setVec3(name + ".diffuse", light.diffuse * localScale);
    shader.setVec3(name + ".specular", light.specular * localScale);
    shader.setFloat(name + ".constant", light.constant);
    shader.setFloat(name + ".linear", light.linear);
    shader.setFloat(name + ".quadratic", light.quadratic);
  }

  const auto &spot = mLights.spotLight;
  shader.setVec3("uSpotLight.position", spot.position);
  shader.setVec3("uSpotLight.direction", spot.direction);
  shader.setVec3("uSpotLight.ambient", spot.ambient * localScale);
  shader.setVec3("uSpotLight.diffuse", spot.diffuse * localScale);
  shader.setVec3("uSpotLight.specular", spot.specular * localScale);
  shader.setFloat("uSpotLight.cutOff", spot.cutOff);
  shader.setFloat("uSpotLight.outerCutOff", spot.outerCutOff);
}
