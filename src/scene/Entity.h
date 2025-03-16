#pragma once
#include "Scene.h"

class Entity {
public:
  Entity(entt::entity handle, Scene *scene);

  template <typename T, typename... Args> T &addComponent(Args &&...args);

  template <typename T> T &getComponent();

  explicit operator bool() const;

private:
  entt::entity mHandle;
  Scene *mScene;
};