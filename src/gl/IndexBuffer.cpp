#include "IndexBuffer.h"

#include <GLFW/glfw3.h>

IndexBuffer::IndexBuffer(const unsigned int *data, const GLsizei count)
    : mCount(count) {
  glCreateBuffers(1, &mRendererId);
  glNamedBufferStorage(mRendererId,
                       static_cast<GLsizeiptr>(count * sizeof(unsigned int)),
                       data, 0);
}

IndexBuffer::~IndexBuffer() { clean(); }

void IndexBuffer::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteBuffers(1, &mRendererId);
    mRendererId = 0;
  }
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept
    : mRendererId(other.mRendererId), mCount(other.mCount) {
  other.mRendererId = 0;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept {
  if (this != &other) {
    clean();
    mRendererId = other.mRendererId;
    mCount = other.mCount;
    other.mRendererId = 0;
  }
  return *this;
}
