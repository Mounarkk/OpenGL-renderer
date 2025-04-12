#pragma once

#include "Scene.h"

#include <entt/entt.hpp>

class Scene; // Forward declaration

class Entity {
public:
  Entity(entt::entity handle, Scene* scene);

  template <typename T, typename... Args>
  T& addComponent(Args&&... args) {
    return mScene->getRegistry().emplace<T>(mHandle, std::forward<Args>(args)...);
  }

  template <typename T>
  T& getComponent() {
    return mScene->getRegistry().get<T>(mHandle);
  }

  explicit operator bool() const;

  [[nodiscard]] entt::entity handle() const { return mHandle; }
  [[nodiscard]] Scene* scene() const { return mScene; }

private:
  entt::entity mHandle;
  Scene* mScene;
};