#pragma once
#include "Scene.h"

#include <entt/entt.hpp>

#include <utility>

/**
 * Lightweight handle to an entity of a Scene.
 *
 * Copying an Entity copies the handle, not the entity. Components live in the
 * scene registry, so a handle must not outlive its scene.
 */
class Entity {
public:
  Entity(entt::entity handle, Scene *scene);

  template <typename T, typename... Args> T &addComponent(Args &&...args) {
    return mScene->getRegistry().emplace<T>(mHandle,
                                            std::forward<Args>(args)...);
  }

  /// The component must exist (asserts in debug builds).
  template <typename T> T &getComponent() {
    return mScene->getRegistry().get<T>(mHandle);
  }

  template <typename T> [[nodiscard]] bool hasComponent() const {
    return mScene->getRegistry().all_of<T>(mHandle);
  }

  /// True if the handle refers to a live entity.
  explicit operator bool() const;

  [[nodiscard]] entt::entity handle() const { return mHandle; }
  [[nodiscard]] Scene *scene() const { return mScene; }

private:
  entt::entity mHandle;
  Scene *mScene;
};
