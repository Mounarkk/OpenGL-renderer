#pragma once
#include <entt/entt.hpp>

class Scene; // Forward declaration

class Entity {
public:
  Entity(entt::entity handle, Scene* scene);

  template <typename T, typename... Args>
  T& addComponent(Args&&... args);

  template <typename T>
  T& getComponent();

  explicit operator bool() const;

  entt::entity handle() const { return mHandle; }
  Scene* scene() const { return mScene; }

private:
  entt::entity mHandle;
  Scene* mScene;
};