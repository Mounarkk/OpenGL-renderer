#include "Entity.h"

Entity::Entity(entt::entity handle, Scene* scene)
: m_Handle(handle), m_Scene(scene) {}

template<typename T, typename... Args>
T& Entity::addComponent(Args&&... args) {
  return m_Scene->GetRegistry().emplace<T>(m_Handle, std::forward<Args>(args)...);
}

template<typename T>
T& Entity::getComponent() {
  return m_Scene->GetRegistry().get<T>(m_Handle);
}