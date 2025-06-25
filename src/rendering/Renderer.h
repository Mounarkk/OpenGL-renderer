#pragma once
#include "RenderPass.h"
#include "../scene/Scene.h"

class Renderer {
public:
  virtual ~Renderer() = default;
  static void clear();
  void prepareCommandQueue(Scene &scene);
  void flush(const glm::mat4 &projMat, const glm::mat4 &viewMat);
  void cleanup() const;
  void resize(int width, int height) const;
protected:
  std::vector<RenderCommand> s_CommandQueue;
  std::vector<RenderPass*> mRenderPasses;
};

class ForwardRenderer final : public Renderer {
public:
  ForwardRenderer();
  ~ForwardRenderer() override;
};