#pragma once
#include "../rendering/Renderer.h"
#include "../scene/Components.h"
#include "../scene/Scene.h"

class RenderingSystem {
public:
  static void onUpdate(Scene &scene, const glm::mat4 &viewProj) {
    auto view = scene.getAll<Transform, MeshRenderer>();
    for (auto entity : view) {
      auto &transform = view.get<Transform>(entity);
      auto &meshRenderer = view.get<MeshRenderer>(entity);
      Renderer::submit({transform.getWorldMatrix(), meshRenderer.mesh,
                        meshRenderer.material});
    }
    Renderer::flush(viewProj);
  }
};