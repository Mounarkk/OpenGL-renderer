#include "RenderPass.h"
#include "../core/Config.h"
#include "../core/Logger.h"
#include "../resource/ResourceManager.h"
#include "LightManager.h"

#include <gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {
/// Blend between logarithmic (1.0) and uniform (0.0) cascade splits.
constexpr float kSplitLambda = 0.8f;

/// How far behind a cascade, towards the light, occluders are still captured,
/// as a multiple of the cascade radius.
constexpr float kCasterDistanceFactor = 4.0f;

std::string indexed(const char *name, const size_t index) {
  return std::string(name) + "[" + std::to_string(index) + "]";
}

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

ShadowMappingPass::ShadowMappingPass(const int mapSize, const int cascadeCount)
    : mMapSize(mapSize),
      mCascadeCount(std::clamp(cascadeCount, 1, kMaxCascades)) {
  const std::string dir = Config::getShaderPath();
  mShader = ResourceManager::loadShader(dir + "shadowm_shader.vert",
                                        dir + "shadowm_shader.frag",
                                        dir + "shadowm_shader.geom");
  mFBO = std::make_unique<FrameBuffer>(
      mMapSize, mMapSize, FrameBufferType::CascadedShadowMap, mCascadeCount);

  mShadowData.shadowMapArray = mFBO->getDepthTexture();
  mShadowData.cascadeCount = mCascadeCount;
}

void ShadowMappingPass::updateCascades(const FrameContext &frame) {
  mShadowData.lightSpaceMatrices.clear();
  mShadowData.cascadeFarPlanes.clear();
  mShadowData.cascadeTexelSizes.clear();
  mShadowData.cascadeTexelDepths.clear();

  // "Practical split scheme": logarithmic splits give each cascade the same
  // perspective aliasing, uniform splits avoid tiny first cascades. Blend both.
  const float n = frame.nearPlane;
  const float f = frame.farPlane;
  float sliceNear = n;
  for (int i = 1; i <= mCascadeCount; ++i) {
    const float p = static_cast<float>(i) / static_cast<float>(mCascadeCount);
    const float logSplit = n * std::pow(f / n, p);
    const float uniformSplit = n + (f - n) * p;
    const float sliceFar =
        kSplitLambda * logSplit + (1.0f - kSplitLambda) * uniformSplit;

    float texelSize = 0.0f;
    float texelDepth = 0.0f;
    mShadowData.lightSpaceMatrices.push_back(computeCascadeMatrix(
        frame, sliceNear, sliceFar, texelSize, texelDepth));
    mShadowData.cascadeFarPlanes.push_back(sliceFar);
    mShadowData.cascadeTexelSizes.push_back(texelSize);
    mShadowData.cascadeTexelDepths.push_back(texelDepth);
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

  const glm::vec3 lightDir =
      LightManager::getInstance().getLights().directionalLight.direction;
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
  updateCascades(frame);

  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  // Occluders between the light and the near plane are clamped to depth 0
  // instead of being clipped, so they still cast shadows ("pancaking").
  glEnable(GL_DEPTH_CLAMP);
  glClear(GL_DEPTH_BUFFER_BIT);

  mShader->use();
  mShader->setInt("uCascadeCount", mCascadeCount);
  for (size_t i = 0; i < mShadowData.lightSpaceMatrices.size(); ++i)
    mShader->setMat4(indexed("uLightSpaceMatrices", i),
                     mShadowData.lightSpaceMatrices[i]);

  // Alpha tested materials (foliage, fences, nets) must cut their shadows too
  constexpr int albedoUnit = 1;
  for (const auto &[model, mesh, material] : commands) {
    material->bindAlbedo(*mShader, "uAlbedoMap", albedoUnit);
    mShader->setMat4("uModel", model);
    mesh->draw();
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
}

void ForwardLightingPass::execute(const std::vector<RenderCommand> &commands,
                                  const FrameContext &frame) {
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mShader->use();
  mShader->setMat4("uViewProj", frame.projection * frame.view);
  mShader->setMat4("uView", frame.view);
  mShader->setVec3("uViewPos", frame.cameraPosition);
  mShader->setBool("uShowCascades", mShowCascades);

  // Texture unit 0 is reserved for the shadow map, see Material
  const bool hasShadows = mShadowData && mShadowData->cascadeCount > 0;
  mShader->setInt("uCascadeCount", hasShadows ? mShadowData->cascadeCount : 0);
  if (hasShadows) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mShadowData->shadowMapArray);
    mShader->setInt("uShadowMap", 0);
    for (int i = 0; i < mShadowData->cascadeCount; ++i) {
      mShader->setMat4(indexed("uLightSpaceMatrices", i),
                       mShadowData->lightSpaceMatrices[i]);
      mShader->setFloat(indexed("uCascadeFarPlanes", i),
                        mShadowData->cascadeFarPlanes[i]);
      mShader->setFloat(indexed("uCascadeTexelSizes", i),
                        mShadowData->cascadeTexelSizes[i]);
      mShader->setFloat(indexed("uCascadeTexelDepths", i),
                        mShadowData->cascadeTexelDepths[i]);
    }
  }

  LightManager::getInstance().bindLights(*mShader);

  // Commands are sorted by material, skip redundant binds
  const Material *boundMaterial = nullptr;
  for (const auto &[model, mesh, material] : commands) {
    if (material.get() != boundMaterial) {
      material->bind(*mShader);
      boundMaterial = material.get();
    }
    mShader->setMat4("uModel", model);
    mesh->draw();
  }

  mSkybox->render(frame.projection, frame.view);

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

  mQuadVAO = std::make_unique<VertexArray>();
  mQuadVBO = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 2);
  layout.push(GL_FLOAT, 2);
  mQuadVAO->addBuffer(*mQuadVBO, layout);
  VertexArray::unbind();
}

void PostProcessingPass::execute(const std::vector<RenderCommand> &,
                                 const FrameContext &) {
  FrameBuffer::unbind();
  glViewport(0, 0, mWidth, mHeight);
  glDisable(GL_DEPTH_TEST);

  mShader->use();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mSourceTexture);
  mShader->setInt("screenTexture", 0);

  mQuadVAO->bind();
  glDrawArrays(GL_TRIANGLES, 0, 6);
  VertexArray::unbind();

  glEnable(GL_DEPTH_TEST);
}

void PostProcessingPass::resize(const int width, const int height) {
  mWidth = width;
  mHeight = height;
}
