#pragma once
#include "Components.h"
#include <entt/entt.hpp>

class Entity; // Forward declaration

class Scene {
public:
  Scene() = default;

  Entity createEntity(const std::string &name = "Entity");

  entt::registry &getRegistry();

  template <typename... Component>
  auto getAll() {
      return m_Registry.view<Component...>();
    }


  void onUpdate(float dt); // Update systems

private:
  entt::registry m_Registry;
};