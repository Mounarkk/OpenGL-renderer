#include "Renderer.h"
#include "../core/Config.h"
#include "../core/Logger.h"

#include <algorithm>

using namespace ShaderInterface;

Renderer::Renderer()
    : mFrameBuffer(sizeof(FrameUniforms)), mLightBuffer(sizeof(LightUniforms)) {
}

void Renderer::submit(Scene &scene) {
  const auto view = scene.getAll<Transform, MeshRenderer>();
  for (const auto entity : view) {
    const auto &[transform, renderer] =
        view.get<Transform, MeshRenderer>(entity);
    if (!renderer.mesh || !renderer.material)
      continue;

    const glm::mat4 model = transform.getWorldMatrix();
    mCommandQueue.push_back({model,
                             renderer.mesh->getBounds().transformed(model),
                             renderer.mesh, renderer.material});
  }

  collectLights(scene);
}

void Renderer::collectLights(Scene &scene) {
  mLights.counts = glm::ivec4(0);

  // The first directional light is the sun
  for (const auto entity : scene.getAll<DirectionalLight>()) {
    const auto &sun = scene.getRegistry().get<DirectionalLight>(entity);
    mLights.sun.direction = glm::vec4(sun.getDirection(), 0.0f);
    mLights.sun.radiance = glm::vec4(sun.color * sun.intensity, 0.0f);
    mLights.sun.ambient = glm::vec4(sun.ambient, 0.0f);
    mLights.counts.z = 1;
    break;
  }

  if (!mSettings.localLightsEnabled)
    return;

  // Lights beyond the block capacity are dropped, with a warning once
  static bool warned = false;
  const auto warnOnce = [](const char *type, const int max) {
    if (!warned)
      Logger::get()->warn("More than {} {} lights, the others are ignored", max,
                          type);
    warned = true;
  };

  const auto points = scene.getAll<Transform, PointLight>();
  for (const auto entity : points) {
    if (mLights.counts.x == kMaxPointLights) {
      warnOnce("point", kMaxPointLights);
      break;
    }
    const auto &[transform, light] = points.get<Transform, PointLight>(entity);
    auto &uniforms = mLights.pointLights[mLights.counts.x++];
    uniforms.positionRange = glm::vec4(transform.position, light.range);
    uniforms.radiance = glm::vec4(light.color * light.intensity, 0.0f);
  }

  const auto spots = scene.getAll<Transform, SpotLight>();
  for (const auto entity : spots) {
    if (mLights.counts.y == kMaxSpotLights) {
      warnOnce("spot", kMaxSpotLights);
      break;
    }
    const auto &[transform, light] = spots.get<Transform, SpotLight>(entity);
    auto &uniforms = mLights.spotLights[mLights.counts.y++];
    uniforms.positionRange = glm::vec4(transform.position, light.range);
    uniforms.directionOuterCos = glm::vec4(
        transform.getForward(), std::cos(glm::radians(light.outerAngle)));
    uniforms.radianceInnerCos =
        glm::vec4(light.color * light.intensity,
                  std::cos(glm::radians(light.innerAngle)));
  }
}

FrameContext Renderer::beginFrame(const FrameContext &cameraFrame) {
  FrameContext frame = cameraFrame;
  frame.settings = mSettings;
  frame.lights = &mLights;

  FrameUniforms uniforms{};
  uniforms.view = frame.view;
  uniforms.projection = frame.projection;
  uniforms.viewProjection = frame.projection * frame.view;
  uniforms.cameraPosition = glm::vec4(frame.cameraPosition, 1.0f);
  uniforms.viewport = {0.0f, 0.0f, frame.nearPlane, frame.farPlane};
  mFrameBuffer.update(&uniforms, sizeof(uniforms));

  // The tail of the spot light array is unused, no need to upload it
  const auto lightBytes = static_cast<GLsizeiptr>(
      offsetof(LightUniforms, spotLights) +
      sizeof(SpotLightUniforms) * static_cast<size_t>(mLights.counts.y));
  mLightBuffer.update(&mLights, lightBytes);

  mFrameBuffer.bindBase(kFrameBlock);
  mLightBuffer.bindBase(kLightsBlock);

  mStats.pointLights = mLights.counts.x;
  mStats.spotLights = mLights.counts.y;
  return frame;
}

ForwardRenderer::ForwardRenderer(const int width, const int height) {
  mShadowPass = std::make_unique<ShadowMappingPass>(Config::getShadowMapSize());
  mLightingPass = std::make_unique<ForwardLightingPass>(width, height);
  mPostProcessingPass = std::make_unique<PostProcessingPass>();
  mPostProcessingPass->resize(width, height);
}

void ForwardRenderer::render(const FrameContext &cameraFrame) {
  const FrameContext frame = beginFrame(cameraFrame);

  // Grouping by material limits texture rebinds in the lighting pass
  std::sort(mCommandQueue.begin(), mCommandQueue.end(),
            [](const RenderCommand &a, const RenderCommand &b) {
              return a.material < b.material;
            });

  mShadowPass->execute(mCommandQueue, frame);

  mLightingPass->setShadowMap(mShadowPass->getShadowMap());
  mLightingPass->execute(mCommandQueue, frame);

  mPostProcessingPass->setSourceTexture(mLightingPass->getColorTexture());
  mPostProcessingPass->execute(mCommandQueue, frame);

  mStats.submitted = mCommandQueue.size();
  mStats.drawnShadow = mShadowPass->getDrawnCount();
  mStats.drawnLighting = mLightingPass->getDrawnCount();
  mCommandQueue.clear();
}

void ForwardRenderer::resize(const int width, const int height) {
  if (width <= 0 || height <= 0)
    return;
  mLightingPass->resize(width, height);
  mPostProcessingPass->resize(width, height);
}
