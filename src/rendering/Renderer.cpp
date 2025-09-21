#include "Renderer.h"

#include "LightManager.h"
#include <memory>

Renderer::Renderer() {
  // Base renderer constructor - derived classes will set up their render passes
}

void Renderer::clear() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::prepareCommandQueue(Scene &scene) {
  const auto view = scene.getAll<Transform, MeshRenderer>();

  for (const auto entity : view) {
    auto [transform, meshRenderer] = view.get<Transform, MeshRenderer>(entity);
    RenderCommand renderCommand;
    renderCommand.transform = transform;
    renderCommand.mesh = meshRenderer.mesh;
    renderCommand.material = meshRenderer.material;
    s_CommandQueue.push_back(renderCommand);
  }
}

void Renderer::flush(const glm::mat4 &projMat, const glm::mat4 &viewMat) {
  // Sort by material to minimize state changes
  std::sort(s_CommandQueue.begin(), s_CommandQueue.end(),
            [](const RenderCommand &a, const RenderCommand &b) {
              return a.material < b.material;
            });

  for (const auto &renderPass : mRenderPasses) {
    renderPass->execute(s_CommandQueue, projMat, viewMat);
  }

  s_CommandQueue.clear();
}

ForwardRenderer::ForwardRenderer() {
  // Create render passes using smart pointers for automatic memory management
  auto forward = std::make_unique<ForwardLightingPass>(
      "../res/shaders/forward_shader.vert",
      "../res/shaders/forward_shader.frag", 800, 600);

  // Get the texture ID before moving the forward pass
  const int textColorBufferID = forward->getTextColorBufferID();

  auto postProcessing = std::make_unique<PostProcessingPass>(
      "../res/shaders/screen.vert", "../res/shaders/screen.frag",
      textColorBufferID);

  // Move the unique_ptrs into the render passes vector (cast to base class)
  mRenderPasses.push_back(std::unique_ptr<RenderPass>(forward.release()));
  mRenderPasses.push_back(
      std::unique_ptr<RenderPass>(postProcessing.release()));
}

ForwardRenderer::~ForwardRenderer() { cleanup(); }

void Renderer::cleanup() const {
  // Clean up render pass resources (smart pointers handle memory automatically)
  for (const auto &pass : mRenderPasses) {
    pass->cleanup();
  }
  // Note: std::unique_ptr automatically deletes render passes when vector is
  // destroyed
}

void Renderer::resize(int width, int height) const {
  // Default implementation
  for (const auto &pass : mRenderPasses) {
    pass->resize(width, height);
  }
}
