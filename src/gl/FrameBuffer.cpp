#include "FrameBuffer.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"

#include <GLFW/glfw3.h>

#include <string>

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

void setDepthSampling(const GLuint texture) {
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, kBorderColor);
}
} // namespace

FrameBuffer::FrameBuffer(const int width, const int height,
                         const FrameBufferType type, const int layers)
    : mWidth(width), mHeight(height), mLayers(layers), mType(type) {
  glCreateFramebuffers(1, &mRendererID);

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

  const GLenum status =
      glCheckNamedFramebufferStatus(mRendererID, GL_FRAMEBUFFER);
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
  glCreateTextures(GL_TEXTURE_2D, 1, &mColorTextureID);
  glTextureStorage2D(mColorTextureID, 1, GL_RGBA16F, mWidth, mHeight);
  glTextureParameteri(mColorTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(mColorTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(mColorTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(mColorTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glNamedFramebufferTexture(mRendererID, GL_COLOR_ATTACHMENT0, mColorTextureID,
                            0);

  glCreateRenderbuffers(1, &mDepthRboID);
  glNamedRenderbufferStorage(mDepthRboID, GL_DEPTH24_STENCIL8, mWidth, mHeight);
  glNamedFramebufferRenderbuffer(mRendererID, GL_DEPTH_STENCIL_ATTACHMENT,
                                 GL_RENDERBUFFER, mDepthRboID);
}

void FrameBuffer::createShadowMapAttachments() {
  glCreateTextures(GL_TEXTURE_2D, 1, &mDepthTextureID);
  glTextureStorage2D(mDepthTextureID, 1, GL_DEPTH_COMPONENT32F, mWidth,
                     mHeight);
  setDepthSampling(mDepthTextureID);
  glNamedFramebufferTexture(mRendererID, GL_DEPTH_ATTACHMENT, mDepthTextureID,
                            0);

  glNamedFramebufferDrawBuffer(mRendererID, GL_NONE);
  glNamedFramebufferReadBuffer(mRendererID, GL_NONE);
}

void FrameBuffer::createCascadedShadowMapAttachments() {
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mDepthTextureID);
  glTextureStorage3D(mDepthTextureID, 1, GL_DEPTH_COMPONENT32F, mWidth, mHeight,
                     mLayers);
  setDepthSampling(mDepthTextureID);

  // Attaching the whole array makes the framebuffer layered: the geometry
  // shader picks the destination layer through gl_Layer.
  glNamedFramebufferTexture(mRendererID, GL_DEPTH_ATTACHMENT, mDepthTextureID,
                            0);

  glNamedFramebufferDrawBuffer(mRendererID, GL_NONE);
  glNamedFramebufferReadBuffer(mRendererID, GL_NONE);
}
