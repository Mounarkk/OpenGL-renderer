#pragma once
#include "Components.h"
#include <entt/entt.hpp>

class Entity; // Forward declaration

/**
 * Manages the game world using an Entity-Component-System (ECS) architecture.
 *
 * The Scene class serves as the main container for all game objects (entities)
 * and their associated data (components). It uses the EnTT library to provide
 * efficient entity management and component storage. The scene handles entity
 * creation, component queries, and system updates.
 *
 * Key responsibilities:
 * - Entity lifecycle management (creation, destruction)
 * - Component storage and efficient queries
 * - System coordination and updates
 * - Scene-wide operations and queries
 */
class Scene {
public:
  /**
   * Default constructor that initializes an empty scene.
   */
  Scene() = default;

  /**
   * Creates a new entity in the scene with an optional name.
   * @param name Optional name for the entity (defaults to "Entity")
   * @return Entity wrapper for the newly created entity
   */
  Entity createEntity(const std::string &name = "Entity");

  /**
   * Gets direct access to the underlying EnTT registry.
   * Use this for advanced ECS operations and custom queries.
   * @return Reference to the internal EnTT registry
   */
  entt::registry &getRegistry();

  /**
   * Gets all entities that have the specified components.
   * This is a template function that returns a view for efficient iteration.
   * @tparam Component Types of components to query for
   * @return EnTT view containing all entities with the specified components
   */
  template <typename... Component> auto getAll() {
    return m_Registry.view<Component...>();
  }

  /**
   * Updates all systems in the scene.
   * This should be called once per frame to update game logic.
   * @param dt Delta time since the last update in seconds
   */
  void onUpdate(float dt);

private:
  entt::registry m_Registry;
};