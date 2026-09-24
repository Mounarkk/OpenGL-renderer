#pragma once
#include "../scene/Scene.h"
#include "RenderPass.h"

#include <memory>
#include <vector>

/**
 * Base class of the renderers.
 *
 * A frame is built in two steps: submit() gathers every entity that has a
 * Transform and a MeshRenderer into a command queue, then render() runs the
 * passes of the concrete renderer on that queue. Keeping the two apart makes
 * room for other pipelines (deferred, clustered) using the same scene.
 */
class Renderer {
public:
  virtual ~Renderer() = default;

  /// Queues every renderable entity of the scene for the next render().
  void submit(Scene &scene);

  /// Draws the queued commands to the window, then empties the queue.
  virtual void render(const FrameContext &frame) = 0;

  /// Recreates the size dependent targets. Ignores zero sizes (minimized).
  virtual void resize(int width, int height) = 0;

  /// Debug view coloring each shadow cascade.
  virtual void setShowCascades(bool show) = 0;

protected:
  std::vector<RenderCommand> mCommandQueue;
};

/// Shadow maps, forward lighting, then post processing to the window.
class ForwardRenderer final : public Renderer {
public:
  ForwardRenderer(int width, int height);

  void render(const FrameContext &frame) override;
  void resize(int width, int height) override;
  void setShowCascades(bool show) override;

private:
  std::unique_ptr<ShadowMappingPass> mShadowPass;
  std::unique_ptr<ForwardLightingPass> mLightingPass;
  std::unique_ptr<PostProcessingPass> mPostProcessingPass;
};
