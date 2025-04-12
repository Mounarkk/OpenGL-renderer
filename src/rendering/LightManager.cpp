#include "LightManager.h"
#define DEFAULT_NB_POINT_LIGHTS 4

LightManager& LightManager::getInstance() {
  static LightManager instance; // Thread-safe, one-time init
  return instance;
}

LightManager::LightManager() {

  // Point lights parameters
  constexpr glm::vec3 pointLightPositions[DEFAULT_NB_POINT_LIGHTS] = {
    glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

  for (auto pointLightPosition : pointLightPositions) {
    PointLight pointLight;
    pointLight.position = pointLightPosition;
    mLights.pointLights.push_back(pointLight);
  }

  // Directional light and spotlight will be initialized to their default state
}

LightData LightManager::getLights() { return mLights; }

void LightManager::bindLights(const std::shared_ptr<Shader> &shader) const {
  // Bind directional light
  shader->setVec3("uDirLight.direction", mLights.directionalLight.direction);
  shader->setVec3("uDirLight.ambient", mLights.directionalLight.ambient);
  shader->setVec3("uDirLight.diffuse", mLights.directionalLight.diffuse);
  shader->setVec3("uDirLight.specular", mLights.directionalLight.specular);

  // Bind point lights
  for (int i = 0; i < 4; i++) {
    std::ostringstream oss;
    oss << "uPointLights[" << i << "]";
    shader->setVec3(oss.str() + ".position", mLights.pointLights[i].position);
    shader->setVec3(oss.str() + ".ambient", mLights.pointLights[i].ambient);
    shader->setVec3(oss.str() + ".diffuse", mLights.pointLights[i].diffuse);
    shader->setVec3(oss.str() + ".specular", mLights.pointLights[i].specular);
    shader->setFloat(oss.str() + ".constant", mLights.pointLights[i].constant);
    shader->setFloat(oss.str() + ".linear", mLights.pointLights[i].linear);
    shader->setFloat(oss.str() + ".quadratic", mLights.pointLights[i].quadratic);
  }

  // Bind spotlights
  shader->setVec3("uSpotLight.ambient", mLights.spotLight.ambient);
  shader->setVec3("uSpotLight.diffuse", mLights.spotLight.diffuse);
  shader->setVec3("uSpotLight.specular", mLights.spotLight.specular);
  shader->setVec3("uSpotLight.position", mLights.spotLight.position);
  shader->setVec3("uSpotLight.direction", mLights.spotLight.direction);
  shader->setFloat("uSpotLight.cutOff", mLights.spotLight.cutOff);
  shader->setFloat("uSpotLight.outerCutOff", mLights.spotLight.outerCutOff);
}

