#include "LightManager.h"
#define DEFAULT_NB_POINT_LIGHTS 4

LightManager::LightManager() {
  LightData lightData;

  // Point lights parameters
  constexpr glm::vec3 pointLightPositions[DEFAULT_NB_POINT_LIGHTS] = {
    glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

  for (auto pointLightPosition : pointLightPositions) {
    PointLight pointLight;
    pointLight.position = pointLightPosition;
    lightData.pointLights.push_back(pointLight);
  }
}

LightManager *LightManager::getInstance() {
  if (pInstancePtr == nullptr) {
    std::lock_guard lock(mMtx);
    if (pInstancePtr == nullptr) {
      pInstancePtr = new LightManager();
    }
  }
  return pInstancePtr;
}

LightData LightManager::getLights() { return mLights; }

