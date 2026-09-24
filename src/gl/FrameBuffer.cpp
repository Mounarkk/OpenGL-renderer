#include "FrameBuffer.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"

#include <GLFW/glfw3.h>

#include <utility>

namespace {
constexpr float kBorderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

const char *typeName(const FrameBufferType type) {
  switch (type) {
  case FrameBufferType::Color:
    return "Color";
  case FrameBufferType::ShadowMap:
    return "ShadowMap";
  case FrameBufferType::CascadedShadowMap:
    return "CascadedShadowMap";
  }
  return "Unknown";
}
} // namespace

FrameBuffer::FrameBuffer(const int width, const int height,
                         const FrameBufferType type, const int layers)
    : mWidth(width), mHeight(height), mLayers(layers), mType(type) {
  glGenFramebuffers(1, &mRendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);

  switch (mType) {
  case FrameBufferType::Color:
    createColorAttachments();
    break;
  case FrameBufferType::ShadowMap:
    createShadowMapAttachments();
    break;
  case FrameBufferType::CascadedShadowMap:
    createCascadedShadowMapAttachments();
    break;
  }

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    clean();
    throw RendererException(std::string("Incomplete ") + typeName(type) +
                            " framebuffer");
  }
  Logger::get()->debug("FrameBuffer created ({}, {}x{}x{})", typeName(type),
                       width, height, layers);
}

FrameBuffer::~FrameBuffer() { clean(); }

void FrameBuffer::clean() {
  if (!glfwGetCurrentContext())
    return;

  if (mColorTextureID != 0)
    glDeleteTextures(1, &mColorTextureID);
  if (mDepthTextureID != 0)
    glDeleteTextures(1, &mDepthTextureID);
  if (mDepthRboID != 0)
    glDeleteRenderbuffers(1, &mDepthRboID);
  if (mRendererID != 0)
    glDeleteFramebuffers(1, &mRendererID);
  release();
}

void FrameBuffer::release() noexcept {
  mRendererID = 0;
  mColorTextureID = 0;
  mDepthTextureID = 0;
  mDepthRboID = 0;
}

FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
    : mRendererID(other.mRendererID), mWidth(other.mWidth),
      mHeight(other.mHeight), mLayers(other.mLayers), mType(other.mType),
      mColorTextureID(other.mColorTextureID),
      mDepthTextureID(other.mDepthTextureID), mDepthRboID(other.mDepthRboID) {
  other.release();
}

FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept {
  if (this != &other) {
    clean();
    mRendererID = other.mRendererID;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mLayers = other.mLayers;
    mType = other.mType;
    mColorTextureID = other.mColorTextureID;
    mDepthTextureID = other.mDepthTextureID;
    mDepthRboID = other.mDepthRboID;
    other.release();
  }
  return *this;
}

void FrameBuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
  glViewport(0, 0, mWidth, mHeight);
}

void FrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FrameBuffer::createColorAttachments() {
  glGenTextures(1, &mColorTextureID);
  glBindTexture(GL_TEXTURE_2D, mColorTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         mColorTextureID, 0);

  glGenRenderbuffers(1, &mDepthRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, mDepthRboID);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, mDepthRboID);
}

void FrameBuffer::createShadowMapAttachments() {
  glGenTextures(1, &mDepthTextureID);
  glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, mWidth, mHeight, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, kBorderColor);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         mDepthTextureID, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
}

void FrameBuffer::createCascadedShadowMapAttachments() {
  glGenTextures(1, &mDepthTextureID);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTextureID);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, mWidth, mHeight,
               mLayers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, kBorderColor);
  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

  // Attaching the whole array makes the framebuffer layered: the geometry
  // shader picks the destination layer through gl_Layer.
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTextureID, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
}
