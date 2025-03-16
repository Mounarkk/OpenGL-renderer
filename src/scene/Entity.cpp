#include "Entity.h"

Entity::Entity(entt::entity handle, Scene *scene)
    : mHandle(handle), mScene(scene) {}

template <typename T, typename... Args>
T &Entity::addComponent(Args &&...args) {
  return mScene->getRegistry().emplace<T>(mHandle, std::forward<Args>(args)...);
}

template <typename T> T &Entity::getComponent() {
  return mScene->getRegistry().get<T>(mHandle);
}
