#include "Renderer.h"

#include "LightManager.h"

std::vector<RenderCommand> Renderer::s_CommandQueue;

Renderer::Renderer() {
  // Uses pointers to be able to apply polymorphism
  const auto forward =
      new ForwardRenderPass("../res/shaders/default_shader_frag.vert",
                            "../res/shaders/default_shader.frag", 800, 600);
  const auto final =
    new FinalRenderPass("../res/shaders/screen.vert", "../res/shaders/screen.frag", forward
      ->getTextColorBufferID());

  mRenderPasses.push_back(reinterpret_cast<RenderPass *>(forward));
  mRenderPasses.push_back(reinterpret_cast<RenderPass *>(final));
}

Renderer::~Renderer() {
  cleanup();
}



void Renderer::clear() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::submit(const RenderCommand &command) {
  s_CommandQueue.push_back(command);
}

void Renderer::flush(const glm::mat4 &projMat, const glm::mat4 &viewMat) const {
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

void Renderer::cleanup() const {
  for (const auto & pass : mRenderPasses) {
    pass->cleanup();
  }
}

void Renderer::resize(const int width, const int height) const {
  for (const auto & pass : mRenderPasses) {
    pass->resize(width, height);
  }
}
