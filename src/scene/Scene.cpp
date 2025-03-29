#include "Scene.h"
#include "Entity.h"

Entity Scene::createEntity(const std::string &name) {
  const auto entity = m_Registry.create();
  m_Registry.emplace<Tag>(entity, Tag{name});
  return Entity{entity, this};
}

entt::registry &Scene::getRegistry() { return m_Registry; };


void Scene::onUpdate(float dt) {
  // Example: Update all entities with Transform and MeshRenderer components
  const auto view = m_Registry.view<Transform, MeshRenderer>();
  for (const auto entity : view) {
    auto &transform = view.get<Transform>(entity);
    auto &meshRenderer = view.get<MeshRenderer>(entity);

    // Update logic here (e.g., update transform, render mesh)
    // TODO: some logic will be implemented here
  }
}


