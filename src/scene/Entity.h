#pragma once

#include "Scene.h"

#include <entt/entt.hpp>

class Scene; // Forward declaration

/**
 * Entity wrapper for the ECS (Entity-Component-System) architecture.
 * 
 * The Entity class provides a convenient wrapper around EnTT entity handles,
 * offering a more intuitive interface for working with entities and their
 * components. It maintains a reference to the scene that owns the entity
 * and provides type-safe methods for component manipulation. This class
 * follows the handle pattern, where the actual entity data is stored in
 * the scene's registry.
 * 
 * Key features:
 * - Type-safe component addition and retrieval
 * - Automatic scene registry management
 * - Lightweight handle-based design
 * - Integration with EnTT's efficient ECS implementation
 * - Convenient interface for common entity operations
 */
class Entity {
public:
  /**
   * Constructs an entity wrapper from an EnTT handle and scene reference.
   * @param handle EnTT entity handle
   * @param scene Pointer to the scene that owns this entity
   */
  Entity(entt::entity handle, Scene* scene);

  /**
   * Adds a component to this entity with the specified constructor arguments.
   * @tparam T Component type to add
   * @tparam Args Constructor argument types
   * @param args Arguments to forward to the component constructor
   * @return Reference to the newly created component
   */
  template <typename T, typename... Args>
  T& addComponent(Args&&... args) {
    return mScene->getRegistry().emplace<T>(mHandle, std::forward<Args>(args)...);
  }

  /**
   * Gets a component of the specified type from this entity.
   * @tparam T Component type to retrieve
   * @return Reference to the component (throws if component doesn't exist)
   */
  template <typename T>
  T& getComponent() {
    return mScene->getRegistry().get<T>(mHandle);
  }

  /**
   * Checks if this entity is valid (not destroyed).
   * @return true if the entity is valid, false otherwise
   */
  explicit operator bool() const;

  /**
   * Gets the underlying EnTT entity handle.
   * @return EnTT entity handle for advanced operations
   */
  [[nodiscard]] entt::entity handle() const { return mHandle; }
  
  /**
   * Gets the scene that owns this entity.
   * @return Pointer to the owning scene
   */
  [[nodiscard]] Scene* scene() const { return mScene; }

private:
  entt::entity mHandle;
  Scene* mScene;
};