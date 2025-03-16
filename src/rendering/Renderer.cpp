#include "Renderer.h"

void Renderer::submit(const RenderCommand &command) {
  s_CommandQueue.push_back(command);
}

void Renderer::flush(const glm::mat4 &viewProj) {
  // Sort by material to minimize state changes
  std::sort(s_CommandQueue.begin(), s_CommandQueue.end(),
            [](const RenderCommand &a, const RenderCommand &b) {
              return a.material < b.material;
            });

  // Batch draw calls
  for (const auto &[transform, mesh, material] : s_CommandQueue) {
    material->bind();
    material->getShader()->setMat4("uViewProj", viewProj);
    material->getShader()->setMat4("uModel", transform);
    mesh->draw();
  }

  s_CommandQueue.clear();
}