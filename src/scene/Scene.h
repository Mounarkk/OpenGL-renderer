#pragma once
#include "Entity.h"
#include <entt/entt.hpp>

class Scene {
public:
  Entity createEntity(const std::string& name = "Entity");

  template<typename T>
  auto getAll() { return m_Registry.view<T>(); }

  void onUpdate(float dt); // Update systems

private:
  entt::registry m_Registry;
};