#pragma once
#include "RenderPass.h"

class Renderer {
public:
  Renderer();
  ~Renderer();
  static void clear();
  static void submit(const RenderCommand &command);
  void flush(const glm::mat4 &projMat, const glm::mat4 &viewMat) const;
  void cleanup() const;
  void resize(int width, int height) const;
private:
  static std::vector<RenderCommand> s_CommandQueue;
  std::vector<RenderPass*> mRenderPasses;
};