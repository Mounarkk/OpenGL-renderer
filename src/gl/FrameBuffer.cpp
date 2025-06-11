#include "FrameBuffer.h"
#include "../core/Logger.h"
#include "GLFW/glfw3.h"

FrameBuffer::FrameBuffer(const int width, const int height) {
  // Gen the framebuffer and its attachments
  glGenFramebuffers(1, &mRendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);


  // Texture for color attachment
  glGenTextures(1, &mTextureID);
  glBindTexture(GL_TEXTURE_2D, mTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureID, 0);

  // Render buffer object for depth/stencil testing
  glGenRenderbuffers(1, &mRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, mRboID);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Logger::get()->error("Frame buffer is incomplete!");
  } else {
    Logger::get()->info("Frame buffer created!");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
  clean();
}

void FrameBuffer::clean() {
  if (mRendererID != 0 && glfwGetCurrentContext()) {
    glDeleteFramebuffers(1, &mRendererID);
    glDeleteTextures(1, &mTextureID);
    glDeleteRenderbuffers(1, &mRboID);
    mRendererID = 0;
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
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
}

void FrameBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::unbindOther(const unsigned int id) {
  glBindFramebuffer(GL_FRAMEBUFFER, id);
}


