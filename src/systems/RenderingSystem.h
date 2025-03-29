#pragma once
#include "../rendering/Renderer.h"
#include "../scene/Components.h"
#include "../scene/Scene.h"

class RenderingSystem {
public:
  static void onUpdate(Scene &scene) {
    entt::view<Transform, MeshRenderer> view = scene.getAll<Transform, MeshRenderer>();
    for (auto entity : view) {
      Transform &transform = view.get<Transform>(entity);
      MeshRenderer &meshRenderer = view.get<MeshRenderer>(entity);
      Renderer::submit({transform.getWorldMatrix(), meshRenderer.mesh,
                        meshRenderer.material, LightManager::getInstance()->getLights()});
    }
  }
};