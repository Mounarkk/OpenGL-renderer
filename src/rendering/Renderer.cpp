#include "Renderer.h"

#include "LightManager.h"

Renderer::Renderer() {
  // Get configured paths and window dimensions
  std::string shaderPath = Config::getShaderPath();
  int width = Config::getWindowWidth();
  int height = Config::getWindowHeight();
  
  // Uses pointers to be able to apply polymorphism
  const auto forward =
      new ForwardRenderPass(shaderPath + "default_shader_frag.vert",
                            shaderPath + "default_shader.frag", width, height);
  const auto final =
    new FinalRenderPass(shaderPath + "screen.vert", shaderPath + "screen.frag", forward
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
  m_CommandQueue.push_back(command);
}

void Renderer::flush(const glm::mat4 &viewProj) {
  // Sort by material to minimize state changes
  std::sort(m_CommandQueue.begin(), m_CommandQueue.end(),
            [](const RenderCommand &a, const RenderCommand &b) {
              return a.material < b.material;
            });

  for (int i = 0; i < mRenderPasses.size(); i++) {
    mRenderPasses[i]->execute(m_CommandQueue, viewProj);
  }

  m_CommandQueue.clear();
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
