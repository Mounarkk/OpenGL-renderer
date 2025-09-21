#include "Renderer.h"

#include "LightManager.h"

Renderer::Renderer() {
  // Base renderer constructor - derived classes will set up their render passes
}

Renderer::~Renderer() {
  cleanup();
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

  for (const auto mRenderPasse : mRenderPasses) {
    mRenderPasse->execute(s_CommandQueue, projMat, viewMat);
  }

  s_CommandQueue.clear();
}

ForwardRenderer::ForwardRenderer() {
  // Uses pointers to be able to apply polymorphism
  const auto forward =
      new ForwardLightingPass("../res/shaders/forward_shader.vert",
                            "../res/shaders/forward_shader.frag", 800, 600);
  const auto postProcessing =
    new PostProcessingPass("../res/shaders/screen.vert", "../res/shaders/screen.frag", forward
      ->getTextColorBufferID());

  mRenderPasses.push_back(reinterpret_cast<RenderPass *>(forward));
  mRenderPasses.push_back(reinterpret_cast<RenderPass *>(postProcessing));
}

ForwardRenderer::~ForwardRenderer() {
  cleanup();
}

void Renderer::cleanup() const {
  // Default implementation - can be empty or provide common cleanup logic
  for (const auto& pass : mRenderPasses) {
    pass->cleanup();
  }
}

void Renderer::resize(int width, int height) const {
  // Default implementation
  for (const auto& pass : mRenderPasses) {
    pass->resize(width, height);
  }
}
