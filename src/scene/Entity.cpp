#include "Entity.h"

Entity::Entity(const entt::entity handle, Scene *scene)
    : mHandle(handle), mScene(scene) {}

Entity::operator bool() const {
  return mScene != nullptr && mHandle != entt::null &&
         mScene->getRegistry().valid(mHandle);
}
