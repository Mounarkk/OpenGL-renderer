#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <glad/glad.h>
#include <memory>

/**
 * OpenGL framebuffer wrapper for off-screen rendering and post-processing.
 * 
 * The FrameBuffer class encapsulates an OpenGL framebuffer object (FBO) with
 * associated color and depth attachments. This enables rendering to textures
 * instead of the default framebuffer, which is essential for post-processing
 * effects, shadow mapping, and other advanced rendering techniques. The class
 * follows RAII principles with move semantics for safe resource management.
 * 
 * Key features:
 * - Complete framebuffer setup with color and depth attachments
 * - Move-only semantics to prevent resource duplication
 * - Automatic renderbuffer creation for depth testing
 * - Easy binding/unbinding for render target switching
 * - Proper resource cleanup and error handling
 */
class FrameBuffer {
public:
  /**
   * Creates a framebuffer with the specified dimensions.
   * Sets up color texture attachment and depth renderbuffer automatically.
   * @param width Framebuffer width in pixels
   * @param height Framebuffer height in pixels
   */
  FrameBuffer(int width, int height);
  
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
  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;

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
  FrameBuffer& operator=(FrameBuffer&& other) noexcept;

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

  unsigned int mTextureID;  // Color attachment texture ID (public for easy access)
  unsigned int mRboID;      // Depth renderbuffer ID (public for easy access)
private:
  unsigned int mRendererID = 0;
  int mWidth, mHeight;
};