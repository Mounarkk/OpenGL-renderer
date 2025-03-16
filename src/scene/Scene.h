#pragma once
#include "Components.h"
#include "Entity.h"
#include <entt/entt.hpp>

class Scene {
public:
  Entity createEntity(const std::string &name = "Entity");

  entt::registry &getRegistry() { return m_Registry; };

  template <typename T, typename... Args> auto getAll() {
    return m_Registry.view<T, Args>();
  }

  void onUpdate(float dt); // Update systems

private:
  entt::registry m_Registry;
};