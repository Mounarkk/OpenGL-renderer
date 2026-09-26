#include "VertexBuffer.h"

#include <GLFW/glfw3.h>

VertexBuffer::VertexBuffer(const void *data, const GLsizeiptr size) {
  glCreateBuffers(1, &mRendererId);
  // Immutable storage: the size can never change, the driver can place it
  // optimally since no flag allows CPU access afterwards
  glNamedBufferStorage(mRendererId, size, data, 0);
}

VertexBuffer::~VertexBuffer() { clean(); }

void VertexBuffer::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteBuffers(1, &mRendererId);
    mRendererId = 0;
  }
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept
    : mRendererId(other.mRendererId) {
  other.mRendererId = 0;
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept {
  if (this != &other) {
    clean();
    mRendererId = other.mRendererId;
    other.mRendererId = 0;
  }
  return *this;
}
