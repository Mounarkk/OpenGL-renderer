#include "Entity.h"
#include "Scene.h" // Now we have full Scene definition

Entity::Entity(entt::entity handle, Scene* scene)
    : mHandle(handle), mScene(scene) {}

template <typename T, typename... Args>
T& Entity::addComponent(Args&&... args) {
  return mScene->getRegistry().emplace<T>(mHandle, std::forward<Args>(args)...);
}

template <typename T>
T& Entity::getComponent() {
  return mScene->getRegistry().get<T>(mHandle);
}

Entity::operator bool() const {
  return mHandle != entt::null && mScene != nullptr;
}