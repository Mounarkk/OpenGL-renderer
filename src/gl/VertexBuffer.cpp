#include "VertexBuffer.h"

#include "GLFW/glfw3.h"

#include <iostream>

VertexBuffer::VertexBuffer(const void* data, unsigned int size) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
  clean();
}

void VertexBuffer::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteBuffers(1, &mRendererId);
    mRendererId = 0;
  }
}

// Move constructor
VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : mRendererId(other.mRendererId) {
  other.mRendererId = 0; // Invalidate the moved-from object
}

// Move assignment operator
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
  if (this != &other) {
    if (mRendererId != 0) {
      glDeleteBuffers(1, &mRendererId);
    }
    mRendererId = other.mRendererId;
    other.mRendererId = 0; // Invalidate the moved-from object
  }
  return *this;
}

void VertexBuffer::Bind() const {
  glBindBuffer(GL_ARRAY_BUFFER, mRendererId);
}

void VertexBuffer::Unbind() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}