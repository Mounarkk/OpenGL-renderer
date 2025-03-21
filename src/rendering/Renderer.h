#pragma once
#include "../rendering/Renderer.h"
#include "../scene/Components.h"
#include "Material.h"
#include "Mesh.h"

struct RenderCommand {
  glm::mat4 transform;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

class Renderer {
public:
  static void clear();
  static void submit(const RenderCommand &command);
  static void flush(const glm::mat4 &viewProj);

private:
  static std::vector<RenderCommand> s_CommandQueue;
};