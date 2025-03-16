#include "Scene.h"

Entity Scene::createEntity(const std::string &name) {
  // Create a new entity add to it a Tag component
  Entity entity = {m_Registry.create(), this};
  entity.addComponent<Tag>().name = name;

  return entity;
}

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
