#include "Entity.h"
#include "Scene.h" // Now we have full Scene definition

Entity::Entity(entt::entity handle, Scene *scene)
    : mHandle(handle), mScene(scene) {}

Entity::operator bool() const {
  return mHandle != entt::null && mScene != nullptr;
}