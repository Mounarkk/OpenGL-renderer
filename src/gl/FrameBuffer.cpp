#include "FrameBuffer.h"
#include "../core/Logger.h"
#include "GLFW/glfw3.h"

FrameBuffer::FrameBuffer(const int width, const int height, FrameBufferType type)
    : mWidth(width), mHeight(height), mType(type) {
  // Create the framebuffer object that will hold our attachments
  glGenFramebuffers(1, &mRendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);

  // Create attachments based on type
  createAttachments();

  // Verify that the framebuffer is complete and ready for rendering
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Logger::get()->error("FrameBuffer is incomplete!");
  } else {
    const char* typeStr = (mType == FrameBufferType::Color) ? "Color" : "ShadowMap";
    Logger::get()->info("FrameBuffer created ({}, {}x{})", typeStr, width, height);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() { clean(); }

void FrameBuffer::clean() {
  if (mRendererID != 0 && glfwGetCurrentContext()) {
    glDeleteFramebuffers(1, &mRendererID);
    
    if (mColorTextureID != 0) {
      glDeleteTextures(1, &mColorTextureID);
      mColorTextureID = 0;
      mTextureID = 0; // Clear legacy alias
    }
    
    if (mDepthTextureID != 0) {
      glDeleteTextures(1, &mDepthTextureID);
      mDepthTextureID = 0;
    }
    
    if (mDepthRboID != 0) {
      glDeleteRenderbuffers(1, &mDepthRboID);
      mDepthRboID = 0;
      mRboID = 0; // Clear legacy alias
    }
    
    mRendererID = 0;
  }
}

// Move constructor
FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
    : mRendererID(other.mRendererID), mWidth(other.mWidth), mHeight(other.mHeight),
      mType(other.mType), mColorTextureID(other.mColorTextureID),
      mDepthTextureID(other.mDepthTextureID), mDepthRboID(other.mDepthRboID) {
  // Update legacy aliases
  mTextureID = mColorTextureID;
  mRboID = mDepthRboID;
  
  // Invalidate the moved-from object
  other.mRendererID = 0;
  other.mColorTextureID = 0;
  other.mDepthTextureID = 0;
  other.mDepthRboID = 0;
  other.mTextureID = 0;
  other.mRboID = 0;
}

// Move assignment operator
FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept {
  if (this != &other) {
    // Clean up current resources
    clean();
    
    // Move data from other
    mRendererID = other.mRendererID;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mType = other.mType;
    mColorTextureID = other.mColorTextureID;
    mDepthTextureID = other.mDepthTextureID;
    mDepthRboID = other.mDepthRboID;
    
    // Update legacy aliases
    mTextureID = mColorTextureID;
    mRboID = mDepthRboID;
    
    // Invalidate the moved-from object
    other.mRendererID = 0;
    other.mColorTextureID = 0;
    other.mDepthTextureID = 0;
    other.mDepthRboID = 0;
    other.mTextureID = 0;
    other.mRboID = 0;
  }
  return *this;
}

void FrameBuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER,
                    mRendererID); // Make this FBO the active render target
}

void FrameBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER,
                    0); // Return to default framebuffer (screen)
}

void FrameBuffer::unbindOther(const unsigned int id) {
  glBindFramebuffer(GL_FRAMEBUFFER, id); // Bind a specific framebuffer by ID
}

void FrameBuffer::bindDepthTexture(unsigned int textureUnit) const {
  if (mType == FrameBufferType::ShadowMap && mDepthTextureID != 0) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
  }
}

void FrameBuffer::createAttachments() {
  switch (mType) {
    case FrameBufferType::Color:
      createColorAttachments();
      break;
    case FrameBufferType::ShadowMap:
      createShadowMapAttachments();
      break;
  }
}

void FrameBuffer::createColorAttachments() {
  // Create color attachment as a texture
  glGenTextures(1, &mColorTextureID);
  glBindTexture(GL_TEXTURE_2D, mColorTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTextureID, 0);

  // Create depth/stencil attachment as a renderbuffer
  glGenRenderbuffers(1, &mDepthRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, mDepthRboID);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Set legacy aliases for backward compatibility
  mTextureID = mColorTextureID;
  mRboID = mDepthRboID;
}

void FrameBuffer::createShadowMapAttachments() {
  // Create depth texture
  glGenTextures(1, &mDepthTextureID);
  glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  
  // Set texture parameters for shadow mapping
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  // Set border color to white (outside shadow map = no shadow)
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTextureID, 0);
  
  // Disable color buffer reads and writes for depth-only rendering
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
}
