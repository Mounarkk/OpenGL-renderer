#include "VertexBuffer.h"

#include <GLFW/glfw3.h>

VertexBuffer::VertexBuffer(const void *data, const GLsizeiptr size) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
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

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, mRendererId); }

void VertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
