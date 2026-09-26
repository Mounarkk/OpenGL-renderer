#include "UniformBuffer.h"

#include <GLFW/glfw3.h>

UniformBuffer::UniformBuffer(const GLsizeiptr size) : mSize(size) {
  glCreateBuffers(1, &mRendererId);
  // Fixed size, but the content can be replaced with glNamedBufferSubData
  glNamedBufferStorage(mRendererId, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

UniformBuffer::~UniformBuffer() { clean(); }

void UniformBuffer::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteBuffers(1, &mRendererId);
    mRendererId = 0;
  }
}

UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept
    : mRendererId(other.mRendererId), mSize(other.mSize) {
  other.mRendererId = 0;
}

UniformBuffer &UniformBuffer::operator=(UniformBuffer &&other) noexcept {
  if (this != &other) {
    clean();
    mRendererId = other.mRendererId;
    mSize = other.mSize;
    other.mRendererId = 0;
  }
  return *this;
}

void UniformBuffer::update(const void *data, const GLsizeiptr size,
                           const GLintptr offset) const {
  glNamedBufferSubData(mRendererId, offset, size, data);
}

void UniformBuffer::bindBase(const GLuint binding) const {
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, mRendererId);
}
