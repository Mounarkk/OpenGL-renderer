#pragma once
#include <glad/glad.h>

/// Attachment layout of a FrameBuffer.
enum class FrameBufferType {
  Color,            ///< RGBA16F color texture + depth/stencil renderbuffer
  ShadowMap,        ///< Single depth texture
  CascadedShadowMap ///< Depth texture array, one layer per cascade
};

/**
 * Off-screen render target.
 *
 * The attachments depend on the FrameBufferType. The color target is a half
 * float texture so lighting can exceed 1.0 before tone mapping in the post
 * process pass. Depth targets clamp to a white border so that anything
 * outside the light frustum is considered lit.
 *
 * Framebuffers are move-only. Resizing is done by creating a new one.
 */
class FrameBuffer {
public:
  /**
   * @param width Width in pixels
   * @param height Height in pixels
   * @param type Attachment layout
   * @param layers Number of layers, only used by CascadedShadowMap
   */
  FrameBuffer(int width, int height,
              FrameBufferType type = FrameBufferType::Color, int layers = 1);
  ~FrameBuffer();

  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;
  FrameBuffer(FrameBuffer &&other) noexcept;
  FrameBuffer &operator=(FrameBuffer &&other) noexcept;

  /// Deletes the framebuffer and its attachments. Safe to call several times.
  void clean();

  /// Binds the framebuffer and sets the viewport to its size.
  void bind() const;

  /// Binds the default framebuffer (the window).
  static void unbind();

  [[nodiscard]] int getWidth() const { return mWidth; }
  [[nodiscard]] int getHeight() const { return mHeight; }
  [[nodiscard]] int getLayers() const { return mLayers; }
  [[nodiscard]] FrameBufferType getType() const { return mType; }

  /// Color attachment, 0 for depth-only framebuffers.
  [[nodiscard]] GLuint getColorTexture() const { return mColorTextureID; }

  /// Depth attachment, 0 for Color framebuffers (they use a renderbuffer).
  [[nodiscard]] GLuint getDepthTexture() const { return mDepthTextureID; }

private:
  GLuint mRendererID = 0;
  int mWidth = 0;
  int mHeight = 0;
  int mLayers = 1;
  FrameBufferType mType = FrameBufferType::Color;

  GLuint mColorTextureID = 0;
  GLuint mDepthTextureID = 0;
  GLuint mDepthRboID = 0;

  void createColorAttachments();
  void createShadowMapAttachments();
  void createCascadedShadowMapAttachments();
  void release() noexcept;
};
