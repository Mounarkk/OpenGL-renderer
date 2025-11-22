#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <glad/glad.h>
#include <memory>

/**
 * Enumeration for different framebuffer types.
 */
enum class FrameBufferType {
  Color,     // Standard color + depth framebuffer
  ShadowMap  // Depth-only framebuffer for shadow mapping
};

/**
 * OpenGL framebuffer wrapper for off-screen rendering and post-processing.
 *
 * The FrameBuffer class encapsulates an OpenGL framebuffer object (FBO) with
 * different attachment configurations based on the specified type. This enables
 * rendering to textures instead of the default framebuffer, which is essential
 * for post-processing effects, shadow mapping, and other advanced rendering
 * techniques. The class follows RAII principles with move semantics for safe
 * resource management.
 *
 * Supported types:
 * - Color: Standard framebuffer with color texture + depth renderbuffer
 * - ShadowMap: Depth-only framebuffer with depth texture for shadow mapping
 *
 * Key features:
 * - Multiple framebuffer types in one class
 * - Move-only semantics to prevent resource duplication
 * - Type-specific attachment creation
 * - Easy binding/unbinding for render target switching
 * - Proper resource cleanup and error handling
 *
 * Usage examples:
 * ```cpp
 * // Create standard color framebuffer
 * FrameBuffer colorFBO(800, 600, FrameBufferType::Color);
 *
 * // Create shadow map framebuffer
 * FrameBuffer shadowFBO(2048, 2048, FrameBufferType::ShadowMap);
 * ```
 */
class FrameBuffer {
public:
  /**
   * Creates a framebuffer with the specified dimensions and type.
   * @param width Framebuffer width in pixels
   * @param height Framebuffer height in pixels
   * @param type Type of framebuffer to create
   */
  FrameBuffer(int width, int height, FrameBufferType type = FrameBufferType::Color);

  /**
   * Destructor that automatically cleans up all framebuffer resources.
   */
  ~FrameBuffer();

  /**
   * Manually deletes the framebuffer and associated resources.
   * Called automatically by destructor.
   */
  void clean();

  // Move-only semantics - framebuffers should not be copied
  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;

  /**
   * Move constructor for transferring framebuffer ownership.
   * @param other FrameBuffer to move from
   */
  FrameBuffer(FrameBuffer &&other) noexcept;

  /**
   * Move assignment operator for transferring framebuffer ownership.
   * @param other FrameBuffer to move from
   * @return Reference to this object
   */
  FrameBuffer &operator=(FrameBuffer &&other) noexcept;

  /**
   * Binds this framebuffer as the current render target.
   * All subsequent rendering will go to this framebuffer instead of the screen.
   */
  void bind() const;

  /**
   * Unbinds any framebuffer and returns to default framebuffer (screen).
   * Static method that can be called without a framebuffer instance.
   */
  static void unbind();

  /**
   * Unbinds a specific framebuffer by ID.
   * @param id OpenGL framebuffer ID to unbind
   */
  static void unbindOther(unsigned int id);

  /**
   * Gets the framebuffer width.
   * @return The width of this framebuffer
   */
  [[nodiscard]] unsigned int getWidth() const { return mWidth; }

  /**
   * Gets the framebuffer height.
   * @return The height of this framebuffer
   */
  [[nodiscard]] unsigned int getHeight() const { return mHeight; }

  /**
   * Gets the framebuffer type.
   * @return The type of this framebuffer
   */
  [[nodiscard]] FrameBufferType getType() const { return mType; }

  /**
   * Gets the color texture ID (only valid for Color type framebuffers).
   * @return OpenGL texture ID of color attachment, or 0 if not applicable
   */
  [[nodiscard]] unsigned int getColorTexture() const { return mColorTextureID; }

  /**
   * Gets the depth texture ID (only valid for ShadowMap type framebuffers).
   * @return OpenGL texture ID of depth attachment, or 0 if not applicable
   */
  [[nodiscard]] unsigned int getDepthTexture() const { return mDepthTextureID; }

  /**
   * Binds the depth texture to the specified texture unit (for ShadowMap type).
   * @param textureUnit Texture unit to bind to
   */
  void bindDepthTexture(unsigned int textureUnit = 0) const;

  // Legacy public members for backward compatibility
  unsigned int mTextureID = 0;      // Alias for color texture (deprecated, use getColorTexture())
  unsigned int mRboID = 0;          // Depth renderbuffer ID (deprecated)

private:
  unsigned int mRendererID = 0;     // OpenGL framebuffer object ID
  int mWidth, mHeight;              // Framebuffer dimensions
  FrameBufferType mType;            // Type of framebuffer
  
  // Type-specific attachment IDs
  unsigned int mColorTextureID = 0;  // Color attachment texture (Color type)
  unsigned int mDepthTextureID = 0;  // Depth attachment texture (ShadowMap type)
  unsigned int mDepthRboID = 0;      // Depth renderbuffer (Color type)

  /**
   * Creates attachments based on framebuffer type.
   */
  void createAttachments();

  /**
   * Creates color framebuffer attachments (color texture + depth renderbuffer).
   */
  void createColorAttachments();

  /**
   * Creates shadow map attachments (depth texture only).
   */
  void createShadowMapAttachments();
};