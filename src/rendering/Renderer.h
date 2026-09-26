#pragma once
#include "../gl/UniformBuffer.h"
#include "../scene/Scene.h"
#include "RenderPass.h"
#include "RendererSettings.h"
#include "ShaderInterface.h"

#include <memory>
#include <vector>

/**
 * Base class of the renderers.
 *
 * A frame is built in two steps: submit() gathers the renderable entities
 * (Transform + MeshRenderer) and the lights of the scene, then render() runs
 * the passes of the concrete renderer. Keeping the two apart makes room for
 * other pipelines (deferred, clustered) using the same scene.
 *
 * Data shared by every shader of a frame (camera, lights) lives in uniform
 * blocks owned by the renderer and uploaded once per frame.
 */
class Renderer {
public:
  Renderer();
  virtual ~Renderer() = default;

  /// Queues the renderable entities and collects the lights of the scene.
  void submit(Scene &scene);

  /// Draws the queued commands to the window, then empties the queue.
  virtual void render(const FrameContext &frame) = 0;

  /// Recreates the size dependent targets. Ignores zero sizes (minimized).
  virtual void resize(int width, int height) = 0;

  /// Options applied to every following frame.
  RendererSettings &getSettings() { return mSettings; }

  [[nodiscard]] const RenderStats &getStats() const { return mStats; }

protected:
  std::vector<RenderCommand> mCommandQueue;
  ShaderInterface::LightUniforms mLights{};
  RendererSettings mSettings;
  RenderStats mStats;

  /// Uploads the frame and light blocks and binds them to their binding
  /// points. Returns the frame context completed with settings and lights.
  FrameContext beginFrame(const FrameContext &cameraFrame);

private:
  UniformBuffer mFrameBuffer;
  UniformBuffer mLightBuffer;

  void collectLights(Scene &scene);
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
