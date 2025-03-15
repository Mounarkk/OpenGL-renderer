#pragma once
#include "Scene.h"

class Entity {
public:
  Entity(entt::entity handle, Scene* scene);

  template<typename T, typename... Args>
  T& addComponent(Args&&... args);

  template<typename T>
  T& getComponent();

  operator bool() const;

private:
  entt::entity m_Handle;
  Scene* m_Scene;
};