#pragma once
#include "Components.h"

#include <entt/entt.hpp>

#include <string>

class Entity;

/// Container of the entities and their components, backed by an EnTT
/// registry.
class Scene {
public:
  /// Creates an entity with a Tag component holding `name`.
  Entity createEntity(const std::string &name = "Entity");

  /// Destroys every entity, releasing the GPU resources they hold.
  void clear();

  entt::registry &getRegistry() { return m_Registry; }

  /// View over the entities owning all the given components.
  template <typename... Component> auto getAll() {
    return m_Registry.view<Component...>();
  }

  /// Per-frame update hook. The scene is static for now.
  void onUpdate(float deltaTime);

private:
  entt::registry m_Registry;
};
