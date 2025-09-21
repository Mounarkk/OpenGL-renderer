#pragma once
#include "../rendering/Renderer.h"
#include "../scene/Components.h"
#include "../scene/Scene.h"

class RenderingSystem {
public:
  static void onUpdate(Scene &scene, Renderer &renderer) {
    const auto view = scene.getAll<Transform, MeshRenderer>();

    for (const auto entity : view) {
      auto [transform, meshRenderer] = view.get<Transform, MeshRenderer>(entity);
      RenderCommand renderCommand;
      renderCommand.transform = transform;
      renderCommand.mesh = meshRenderer.mesh;
      renderCommand.material = meshRenderer.material;
      renderer.submit(renderCommand);
    }
  }
};