#include "LightManager.h"

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
}

void LightManager::rotateSun(const float angleRadians) {
  const glm::mat4 rotation =
      glm::rotate(glm::mat4(1.0f), angleRadians, glm::vec3(0.0f, 1.0f, 0.0f));
  auto &direction = mLights.directionalLight.direction;
  direction = glm::normalize(glm::vec3(rotation * glm::vec4(direction, 0.0f)));
}

void LightManager::bindLights(const Shader &shader) const {
  const auto &sun = mLights.directionalLight;
  shader.setVec3("uDirLight.direction", sun.direction);
  shader.setVec3("uDirLight.ambient", sun.ambient);
  shader.setVec3("uDirLight.diffuse", sun.diffuse);
  shader.setVec3("uDirLight.specular", sun.specular);

  for (int i = 0; i < kPointLightCount; ++i) {
    const auto &light = mLights.pointLights[i];
    const std::string name = "uPointLights[" + std::to_string(i) + "]";
    shader.setVec3(name + ".position", light.position);
    shader.setVec3(name + ".ambient", light.ambient);
    shader.setVec3(name + ".diffuse", light.diffuse);
    shader.setVec3(name + ".specular", light.specular);
    shader.setFloat(name + ".constant", light.constant);
    shader.setFloat(name + ".linear", light.linear);
    shader.setFloat(name + ".quadratic", light.quadratic);
  }

  const auto &spot = mLights.spotLight;
  shader.setVec3("uSpotLight.position", spot.position);
  shader.setVec3("uSpotLight.direction", spot.direction);
  shader.setVec3("uSpotLight.ambient", spot.ambient);
  shader.setVec3("uSpotLight.diffuse", spot.diffuse);
  shader.setVec3("uSpotLight.specular", spot.specular);
  shader.setFloat("uSpotLight.cutOff", spot.cutOff);
  shader.setFloat("uSpotLight.outerCutOff", spot.outerCutOff);
}
