#include "Renderer.h"
#include "../core/Config.h"

#include <algorithm>

namespace {
constexpr int kCascadeCount = 4;
}

void Renderer::submit(Scene &scene) {
  const auto view = scene.getAll<Transform, MeshRenderer>();
  for (const auto entity : view) {
    const auto &[transform, renderer] =
        view.get<Transform, MeshRenderer>(entity);
    if (renderer.mesh && renderer.material)
      mCommandQueue.push_back(
          {transform.getWorldMatrix(), renderer.mesh, renderer.material});
  }
}

ForwardRenderer::ForwardRenderer(const int width, const int height) {
  mShadowPass = std::make_unique<ShadowMappingPass>(Config::getShadowMapSize(),
                                                    kCascadeCount);
  mLightingPass = std::make_unique<ForwardLightingPass>(width, height);
  mPostProcessingPass = std::make_unique<PostProcessingPass>();
  mPostProcessingPass->resize(width, height);
}

void ForwardRenderer::render(const FrameContext &frame) {
  // Grouping by material limits texture rebinds in the lighting pass
  std::sort(mCommandQueue.begin(), mCommandQueue.end(),
            [](const RenderCommand &a, const RenderCommand &b) {
              return a.material < b.material;
            });

  mShadowPass->execute(mCommandQueue, frame);

  mLightingPass->setShadowData(mShadowPass->getShadowData());
  mLightingPass->execute(mCommandQueue, frame);

  mPostProcessingPass->setSourceTexture(mLightingPass->getColorTexture());
  mPostProcessingPass->execute(mCommandQueue, frame);

  mCommandQueue.clear();
}

void ForwardRenderer::resize(const int width, const int height) {
  if (width <= 0 || height <= 0)
    return;
  mLightingPass->resize(width, height);
  mPostProcessingPass->resize(width, height);
}

void ForwardRenderer::setShowCascades(const bool show) {
  mLightingPass->setShowCascades(show);
}
