#pragma once
#include "Components.h"
#include <entt/entt.hpp>

class Entity; // Forward declaration

class Scene {
public:
  Scene();

  Entity createEntity(const std::string &name = "Entity");

  entt::registry &getRegistry();

  template <typename T, typename... Args> auto getAll();

  void onUpdate(float dt); // Update systems

private:
  entt::registry m_Registry;
};