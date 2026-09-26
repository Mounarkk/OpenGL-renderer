#include "RenderPass.h"
#include "../core/Config.h"
#include "../gl/Debug.h"
#include "../resource/ResourceManager.h"
#include "Primitives.h"

#include <gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

using namespace ShaderInterface;

namespace {
/// How far behind a cascade, towards the light, occluders are still captured,
/// as a multiple of the cascade radius.
constexpr float kCasterDistanceFactor = 4.0f;

std::array<glm::vec3, 8> frustumCornersWorldSpace(const glm::mat4 &proj,
                                                  const glm::mat4 &view) {
  const glm::mat4 inverse = glm::inverse(proj * view);
  std::array<glm::vec3, 8> corners{};
  size_t i = 0;
  for (int x = 0; x < 2; ++x) {
    for (int y = 0; y < 2; ++y) {
      for (int z = 0; z < 2; ++z) {
        const glm::vec4 point =
            inverse *
            glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
        corners[i++] = glm::vec3(point) / point.w;
      }
    }
  }
  return corners;
}
} // namespace

// ---------------------------------------------------------------------------
// ShadowMappingPass
// ---------------------------------------------------------------------------

ShadowMappingPass::ShadowMappingPass(const int mapSize)
    : mUniformBuffer(sizeof(ShadowUniforms)), mMapSize(mapSize) {
  const std::string dir = Config::getShaderPath();
  mShader = ResourceManager::loadShader(dir + "shadowm_shader.vert",
                                        dir + "shadowm_shader.frag",
                                        dir + "shadowm_shader.geom");
  mFBO = std::make_unique<FrameBuffer>(
      mMapSize, mMapSize, FrameBufferType::CascadedShadowMap, kCascadeCount);
}

void ShadowMappingPass::updateCascades(const FrameContext &frame) {
  // "Practical split scheme": logarithmic splits give each cascade the same
  // perspective aliasing, uniform splits avoid tiny first cascades. Blend both.
  const float lambda = frame.settings.cascadeSplitLambda;
  const float n = frame.nearPlane;
  const float f = frame.farPlane;
  float sliceNear = n;
  for (int i = 0; i < kCascadeCount; ++i) {
    const float p = static_cast<float>(i + 1) / kCascadeCount;
    const float logSplit = n * std::pow(f / n, p);
    const float uniformSplit = n + (f - n) * p;
    const float sliceFar = lambda * logSplit + (1.0f - lambda) * uniformSplit;

    float texelSize = 0.0f;
    float texelDepth = 0.0f;
    mUniforms.lightSpaceMatrices[i] =
        computeCascadeMatrix(frame, sliceNear, sliceFar, texelSize, texelDepth);
    mUniforms.cascades[i] = {sliceFar, texelSize, texelDepth, 0.0f};
    sliceNear = sliceFar;
  }
}

glm::mat4 ShadowMappingPass::computeCascadeMatrix(const FrameContext &frame,
                                                  const float nearPlane,
                                                  const float farPlane,
                                                  float &texelSize,
                                                  float &texelDepth) const {
  const glm::mat4 sliceProj =
      glm::perspective(frame.fovY, frame.aspectRatio, nearPlane, farPlane);
  const auto corners = frustumCornersWorldSpace(sliceProj, frame.view);

  glm::vec3 center(0.0f);
  for (const auto &corner : corners)
    center += corner;
  center /= static_cast<float>(corners.size());

  float radius = 0.0f;
  for (const auto &corner : corners)
    radius = std::max(radius, glm::length(corner - center));
  // Quantized so that floating point noise does not change the projection
  radius = std::ceil(radius * 16.0f) / 16.0f;

  const glm::vec3 lightDir = glm::vec3(frame.lights->sun.direction);
  const glm::vec3 up =
      std::abs(lightDir.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
  const glm::mat4 lightView = glm::lookAt(center - lightDir, center, up);

  // The sphere spans [-1 - radius, -1 + radius] along the light view axis.
  // The near plane is pushed towards the light so that occluders outside the
  // camera frustum still cast shadows inside it.
  const float zNear = 1.0f - radius - radius * kCasterDistanceFactor;
  const float zFar = 1.0f + radius;
  glm::mat4 lightProj =
      glm::ortho(-radius, radius, -radius, radius, zNear, zFar);

  // Snap the projection to whole texels: the world origin must land on a
  // texel corner, otherwise moving the camera makes shadow edges crawl.
  const float halfSize = static_cast<float>(mMapSize) * 0.5f;
  const glm::vec4 origin =
      lightProj * lightView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) * halfSize;
  const glm::vec4 rounded = glm::round(origin);
  lightProj[3][0] += (rounded.x - origin.x) / halfSize;
  lightProj[3][1] += (rounded.y - origin.y) / halfSize;

  texelSize = 2.0f * radius / static_cast<float>(mMapSize);
  texelDepth = texelSize / (zFar - zNear);
  return lightProj * lightView;
}

void ShadowMappingPass::execute(const std::vector<RenderCommand> &commands,
                                const FrameContext &frame) {
  const auto &settings = frame.settings;
  const bool enabled = settings.shadowsEnabled && frame.lights->counts.z != 0;

  mUniforms.bias = {settings.shadowBiasConstant, settings.shadowBiasSlope,
                    settings.shadowNormalOffset, 0.0f};
  mUniforms.flags = {enabled, settings.shadowPcf, settings.showCascades, 0};
  if (enabled)
    updateCascades(frame);

  // Published even when disabled, the lighting pass reads the flags
  mUniformBuffer.update(&mUniforms, sizeof(mUniforms));
  mUniformBuffer.bindBase(kShadowBlock);

  mDrawnCount = 0;
  if (!enabled)
    return;

  GLDebugGroup group("Shadow pass");
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  // Occluders between the light and the near plane are clamped to depth 0
  // instead of being clipped, so they still cast shadows ("pancaking").
  glEnable(GL_DEPTH_CLAMP);
  glClear(GL_DEPTH_BUFFER_BIT);

  // The near plane is ignored for the same reason: casters in front of it
  // still land in the map thanks to depth clamping
  std::vector<Frustum> cascades;
  cascades.reserve(kCascadeCount);
  for (const glm::mat4 &lightSpace : mUniforms.lightSpaceMatrices)
    cascades.emplace_back(lightSpace);

  mShader->use();
  for (const auto &command : commands) {
    // One bit per cascade the mesh overlaps, the geometry shader skips the
    // others
    unsigned int mask = (1u << kCascadeCount) - 1u;
    if (settings.frustumCulling) {
      mask = 0;
      for (int i = 0; i < kCascadeCount; ++i)
        if (cascades[i].intersects(command.worldBounds, true))
          mask |= 1u << i;
      if (mask == 0)
        continue;
    }

    // Alpha tested materials (foliage, fences, nets) cut their shadows too
    command.material->bindAlbedo();
    mShader->setMat4("uModel", command.model);
    mShader->setUInt("uCascadeMask", mask);
    command.mesh->draw();
    ++mDrawnCount;
  }

  glDisable(GL_DEPTH_CLAMP);
  FrameBuffer::unbind();
}

// ---------------------------------------------------------------------------
// ForwardLightingPass
// ---------------------------------------------------------------------------

ForwardLightingPass::ForwardLightingPass(const int width, const int height) {
  const std::string dir = Config::getShaderPath();
  mShader = ResourceManager::loadShader(dir + "forward_shader.vert",
                                        dir + "forward_shader.frag");
  mFBO = std::make_unique<FrameBuffer>(width, height);
  mSkybox = std::make_unique<Skybox>(Config::getSkyboxPath());

  mGizmoShader = ResourceManager::loadShader(dir + "light_gizmo.vert",
                                             dir + "light_gizmo.frag");
  mGizmoSphere = Primitives::createSphere(1.0f, 16, 12);
}

void ForwardLightingPass::drawLightGizmos(const FrameContext &frame) const {
  const auto &lights = *frame.lights;

  // Markers show the hue of the light at full brightness, whatever its
  // intensity, so that dim lights stay visible.
  const auto markerColor = [](const glm::vec4 &radiance) {
    const glm::vec3 color(radiance);
    const float peak = std::max({color.r, color.g, color.b});
    return peak > 0.0f ? color / peak : glm::vec3(0.0f);
  };
  const auto drawMarker = [&](const glm::vec3 &position, const float radius,
                              const glm::vec3 &color) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(radius));
    mGizmoShader->setMat4("uModel", model);
    mGizmoShader->setVec3("uColor", color);
    mGizmoSphere->draw();
  };

  mGizmoShader->use();
  for (int i = 0; i < lights.counts.x; ++i) {
    const auto &light = lights.pointLights[i];
    drawMarker(glm::vec3(light.positionRange), 0.12f,
               markerColor(light.radiance));
  }

  // Spot lights get a second, smaller sphere along their direction
  for (int i = 0; i < lights.counts.y; ++i) {
    const auto &spot = lights.spotLights[i];
    const glm::vec3 position(spot.positionRange);
    const glm::vec3 color = markerColor(spot.radianceInnerCos);
    drawMarker(position, 0.12f, color);
    drawMarker(position + glm::vec3(spot.directionOuterCos) * 0.25f, 0.06f,
               color);
  }
}

void ForwardLightingPass::execute(const std::vector<RenderCommand> &commands,
                                  const FrameContext &frame) {
  GLDebugGroup group("Lighting pass");
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Camera, lights and shadow data come from the uniform blocks, only the
  // material and the model matrix change between draws
  mShader->use();
  glBindTextureUnit(kShadowMapUnit, mShadowMap);

  const Frustum view(frame.projection * frame.view);
  const Material *boundMaterial = nullptr;
  mDrawnCount = 0;
  for (const auto &command : commands) {
    if (frame.settings.frustumCulling && !view.intersects(command.worldBounds))
      continue;

    // Commands are sorted by material, skip redundant binds
    if (command.material.get() != boundMaterial) {
      command.material->bind(*mShader);
      boundMaterial = command.material.get();
    }
    mShader->setMat4("uModel", command.model);
    command.mesh->draw();
    ++mDrawnCount;
  }

  if (frame.settings.showLightGizmos)
    drawLightGizmos(frame);

  mSkybox->render();

  FrameBuffer::unbind();
}

void ForwardLightingPass::resize(const int width, const int height) {
  mFBO = std::make_unique<FrameBuffer>(width, height);
}

// ---------------------------------------------------------------------------
// PostProcessingPass
// ---------------------------------------------------------------------------

PostProcessingPass::PostProcessingPass() {
  const std::string dir = Config::getShaderPath();
  mShader =
      ResourceManager::loadShader(dir + "screen.vert", dir + "screen.frag");

  // Two triangles covering the screen: position (xy), texCoords (uv)
  constexpr float quadVertices[] = {-1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                                    0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,
                                    -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                                    1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  mQuadVBO = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));
  mQuadVAO = std::make_unique<VertexArray>();
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 2);
  layout.push(GL_FLOAT, 2);
  mQuadVAO->setVertexBuffer(*mQuadVBO, layout);
}

void PostProcessingPass::execute(const std::vector<RenderCommand> &,
                                 const FrameContext &) {
  GLDebugGroup group("Post-process pass");
  FrameBuffer::unbind();
  glViewport(0, 0, mWidth, mHeight);
  glDisable(GL_DEPTH_TEST);

  mShader->use();
  glBindTextureUnit(kScreenUnit, mSourceTexture);
  mQuadVAO->bind();
  glDrawArrays(GL_TRIANGLES, 0, 6);
  VertexArray::unbind();

  glEnable(GL_DEPTH_TEST);
}

void PostProcessingPass::resize(const int width, const int height) {
  mWidth = width;
  mHeight = height;
}
