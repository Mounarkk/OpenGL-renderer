#pragma once
#include "../gl/FrameBuffer.h"
#include "../gl/Shader.h"
#include "../gl/UniformBuffer.h"
#include "Frustum.h"
#include "Material.h"
#include "Mesh.h"
#include "RendererSettings.h"
#include "ShaderInterface.h"
#include "Skybox.h"

#include <glm.hpp>

#include <memory>
#include <vector>

/// One mesh to draw this frame, with its world matrix and bounds computed.
struct RenderCommand {
  glm::mat4 model;
  AABB worldBounds;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

/// Camera state and options shared by every pass for the current frame.
struct FrameContext {
  glm::mat4 view{1.0f};
  glm::mat4 projection{1.0f};
  glm::vec3 cameraPosition{0.0f};
  float fovY = glm::radians(45.0f); ///< Vertical field of view, radians
  float aspectRatio = 1.0f;
  float nearPlane = 0.1f;
  float farPlane = 100.0f;

  /// Filled by the renderer
  RendererSettings settings;
  const ShaderInterface::LightUniforms *lights = nullptr;
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
 * mesh: the geometry shader runs once per cascade (instancing) and routes
 * the triangle to its layer. Meshes are culled per cascade with a bit mask.
 *
 * Fitting a sphere instead of a box keeps the projection size constant when
 * the camera rotates, and snapping it to the texel grid removes the
 * shimmering of shadow edges when the camera moves.
 *
 * The cascade data is published in the shadow uniform block, read by the
 * lighting pass.
 */
class ShadowMappingPass final : public RenderPass {
public:
  /// @param mapSize Width and height of each cascade, in texels
  explicit ShadowMappingPass(int mapSize);

  void execute(const std::vector<RenderCommand> &commands,
               const FrameContext &frame) override;

  [[nodiscard]] GLuint getShadowMap() const { return mFBO->getDepthTexture(); }

  /// Meshes drawn into at least one cascade during the last execute().
  [[nodiscard]] size_t getDrawnCount() const { return mDrawnCount; }

private:
  std::shared_ptr<Shader> mShader;
  std::unique_ptr<FrameBuffer> mFBO;
  UniformBuffer mUniformBuffer;
  ShaderInterface::ShadowUniforms mUniforms{};
  int mMapSize;
  size_t mDrawnCount = 0;

  void updateCascades(const FrameContext &frame);

  /// Light view-projection for the slice [nearPlane, farPlane] of the camera.
  /// Also returns the world size and the depth range of one texel.
  glm::mat4 computeCascadeMatrix(const FrameContext &frame, float nearPlane,
                                 float farPlane, float &texelSize,
                                 float &texelDepth) const;
};

/**
 * Forward Blinn-Phong lighting into an HDR color target, followed by the
 * light markers and the skybox. Reads the shadow map rendered before.
 */
class ForwardLightingPass final : public RenderPass {
public:
  ForwardLightingPass(int width, int height);

  void execute(const std::vector<RenderCommand> &commands,
               const FrameContext &frame) override;
  void resize(int width, int height) override;

  void setShadowMap(const GLuint texture) { mShadowMap = texture; }

  [[nodiscard]] GLuint getColorTexture() const {
    return mFBO->getColorTexture();
  }

  /// Meshes that passed frustum culling during the last execute().
  [[nodiscard]] size_t getDrawnCount() const { return mDrawnCount; }

private:
  std::shared_ptr<Shader> mShader;
  std::unique_ptr<FrameBuffer> mFBO;
  std::unique_ptr<Skybox> mSkybox;
  GLuint mShadowMap = 0;
  size_t mDrawnCount = 0;

  // Light markers
  std::shared_ptr<Shader> mGizmoShader;
  std::shared_ptr<Mesh> mGizmoSphere;

  void drawLightGizmos(const FrameContext &frame) const;
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
