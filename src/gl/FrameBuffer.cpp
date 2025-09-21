#include "FrameBuffer.h"
#include "../core/Logger.h"
#include "GLFW/glfw3.h"

FrameBuffer::FrameBuffer(const int width, const int height) 
    : mWidth(width), mHeight(height) {
  // Create the framebuffer object that will hold our attachments
  glGenFramebuffers(1, &mRendererID);                    // Generate FBO ID
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);        // Bind for configuration

  // Create color attachment as a texture (so we can sample from it later)
  glGenTextures(1, &mTextureID);                         // Generate texture for color data
  glBindTexture(GL_TEXTURE_2D, mTextureID);              // Bind texture for configuration
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);  // Allocate texture memory
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // Linear filtering for smooth results
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);                       // Unbind texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureID, 0);  // Attach to FBO

  // Create depth/stencil attachment as a renderbuffer (we don't need to sample from it)
  glGenRenderbuffers(1, &mRboID);                        // Generate renderbuffer for depth/stencil
  glBindRenderbuffer(GL_RENDERBUFFER, mRboID);           // Bind for configuration
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  // Allocate depth+stencil storage
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRboID);  // Attach to FBO
  glBindRenderbuffer(GL_RENDERBUFFER, 0);                // Unbind renderbuffer

  // Verify that the framebuffer is complete and ready for rendering
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Logger::get()->error("Frame buffer is incomplete!");
  } else {
    Logger::get()->info("Frame buffer created!");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind framebuffer (return to default)
}

FrameBuffer::~FrameBuffer() {
  clean();
}

void FrameBuffer::clean() {
  if (mRendererID != 0 && glfwGetCurrentContext()) {
    glDeleteFramebuffers(1, &mRendererID);   // Delete the framebuffer object
    glDeleteTextures(1, &mTextureID);        // Delete the color attachment texture
    glDeleteRenderbuffers(1, &mRboID);       // Delete the depth/stencil renderbuffer
    mRendererID = 0;  // Mark as invalid to prevent double-deletion
  }
}

// Move constructor
FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : mRendererID(other.mRendererID) {
  // Invalidate the moved-from object
  other.mRendererID = 0;
  other.mTextureID = 0;
  other.mRboID = 0;
}

// Move assignment operator
FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
  if (this != &other) {
    if (mRendererID != 0) {
      glDeleteBuffers(1, &mRendererID);
    }
    mRendererID = other.mRendererID;
    // Invalidate the moved-from object
    other.mRendererID = 0;
    other.mTextureID = 0;
    other.mRboID = 0;
  }
  return *this;
}

void FrameBuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);  // Make this FBO the active render target
}

void FrameBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Return to default framebuffer (screen)
}

void FrameBuffer::unbindOther(const unsigned int id) {
  glBindFramebuffer(GL_FRAMEBUFFER, id);  // Bind a specific framebuffer by ID
}


