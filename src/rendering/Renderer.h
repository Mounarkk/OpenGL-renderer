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

  /// Options applied to every following frame.
  RendererSettings &getSettings() { return mSettings; }

  /// Number of meshes drawn by the last render().
  [[nodiscard]] size_t getLastDrawCount() const { return mLastDrawCount; }

protected:
  std::vector<RenderCommand> mCommandQueue;
  RendererSettings mSettings;
  size_t mLastDrawCount = 0;
};

/// Shadow maps, forward lighting, then post processing to the window.
class ForwardRenderer final : public Renderer {
public:
  ForwardRenderer(int width, int height);

  void render(const FrameContext &frame) override;
  void resize(int width, int height) override;

private:
  std::unique_ptr<ShadowMappingPass> mShadowPass;
  std::unique_ptr<ForwardLightingPass> mLightingPass;
  std::unique_ptr<PostProcessingPass> mPostProcessingPass;
};
