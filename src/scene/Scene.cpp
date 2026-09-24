#include "Scene.h"
#include "Entity.h"

Entity Scene::createEntity(const std::string &name) {
  const auto entity = m_Registry.create();
  m_Registry.emplace<Tag>(entity, Tag{name});
  return Entity{entity, this};
}

void Scene::clear() { m_Registry.clear(); }

void Scene::onUpdate(float /*deltaTime*/) {}
