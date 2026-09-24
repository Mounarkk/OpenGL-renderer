#pragma once
#include "../gl/FrameBuffer.h"
#include "../gl/Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "Skybox.h"

#include <glm.hpp>

#include <memory>
#include <vector>

/// One mesh to draw this frame, with its world matrix already computed.
struct RenderCommand {
  glm::mat4 model;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

/// Camera state shared by every pass for the current frame.
struct FrameContext {
  glm::mat4 view{1.0f};
  glm::mat4 projection{1.0f};
  glm::vec3 cameraPosition{0.0f};
  float fovY = glm::radians(45.0f); ///< Vertical field of view, radians
  float aspectRatio = 1.0f;
  float nearPlane = 0.1f;
  float farPlane = 100.0f;
};

/// Hard limit, must match MAX_CASCADES in the shaders.
constexpr int kMaxCascades = 8;

/// Everything the lighting pass needs to sample the cascaded shadow map.
struct ShadowData {
  GLuint shadowMapArray = 0;
  int cascadeCount = 0;
  std::vector<glm::mat4> lightSpaceMatrices; ///< World to light clip space
  std::vector<float> cascadeFarPlanes;       ///< View space distances
  std::vector<float> cascadeTexelSizes;      ///< World size of one texel
  /// Depth range covered by one texel-sized distance, in the [0, 1] depth
  /// of the shadow map. Used to express the depth bias in texels.
  std::vector<float> cascadeTexelDepths;
};

/// A step of a renderer's frame.
class RenderPass {
public:
  virtual ~RenderPass() = default;

  virtual void execute(const std::vector<RenderCommand> &commands,
                       const FrameContext &frame) = 0;

  /// Called when the window size changes. Most passes do not care.
  virtual void resize(int /*width*/, int /*height*/) {}
};

/**
 * Renders the directional light's cascaded shadow map.
 *
 * The camera frustum is cut into slices along its depth. Each slice gets its
 * own orthographic light projection fitted around its bounding sphere and a
 * layer of a depth texture array. All layers are filled in a single draw per
 * mesh: the geometry shader duplicates each triangle into every layer.
 *
 * Fitting a sphere instead of a box keeps the projection size constant when
 * the camera rotates, and snapping it to the texel grid removes the
 * shimmering of shadow edges when the camera moves.
 */
class ShadowMappingPass final : public RenderPass {
public:
  /**
   * @param mapSize Width and height of each cascade, in texels
   * @param cascadeCount Number of cascades, at most kMaxCascades
   */
  ShadowMappingPass(int mapSize, int cascadeCount);

  void execute(const std::vector<RenderCommand> &commands,
               const FrameContext &frame) override;

  [[nodiscard]] const ShadowData &getShadowData() const { return mShadowData; }

private:
  std::shared_ptr<Shader> mShader;
  std::unique_ptr<FrameBuffer> mFBO;
  int mMapSize;
  int mCascadeCount;
  ShadowData mShadowData;

  void updateCascades(const FrameContext &frame);

  /// Light view-projection for the slice [nearPlane, farPlane] of the camera.
  glm::mat4 computeCascadeMatrix(const FrameContext &frame, float nearPlane,
                                 float farPlane, float &texelSize,
                                 float &texelDepth) const;
};

/**
 * Forward Blinn-Phong lighting into an HDR color target, followed by the
 * skybox. Shadows come from the ShadowData set before execute().
 */
class ForwardLightingPass final : public RenderPass {
public:
  ForwardLightingPass(int width, int height);

  void execute(const std::vector<RenderCommand> &commands,
               const FrameContext &frame) override;
  void resize(int width, int height) override;

  void setShadowData(const ShadowData &data) { mShadowData = &data; }

  /// Tints each cascade with a different color, for debugging.
  void setShowCascades(const bool show) { mShowCascades = show; }

  [[nodiscard]] GLuint getColorTexture() const {
    return mFBO->getColorTexture();
  }

private:
  std::shared_ptr<Shader> mShader;
  std::unique_ptr<FrameBuffer> mFBO;
  std::unique_ptr<Skybox> mSkybox;
  const ShadowData *mShadowData = nullptr;
  bool mShowCascades = false;
};

/// Draws a texture on a fullscreen quad of the default framebuffer, applying
/// gamma correction.
class PostProcessingPass final : public RenderPass {
public:
  PostProcessingPass();

  void execute(const std::vector<RenderCommand> &commands,
               const FrameContext &frame) override;
  void resize(int width, int height) override;

  void setSourceTexture(const GLuint texture) { mSourceTexture = texture; }

private:
  std::shared_ptr<Shader> mShader;
  std::unique_ptr<VertexArray> mQuadVAO;
  std::unique_ptr<VertexBuffer> mQuadVBO;
  GLuint mSourceTexture = 0;
  int mWidth = 0;
  int mHeight = 0;
};
