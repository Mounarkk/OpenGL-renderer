#include "IndexBuffer.h"

#include "GLFW/glfw3.h"

#include <iostream>

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
    : mCount(count) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
  clean();
}

void IndexBuffer::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteBuffers(1, &mRendererId);
    mRendererId = 0;
  }
}


// Move constructor
IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : mRendererId(other.mRendererId), mCount(other.mCount) {
  other.mRendererId = 0; // Invalidate the moved-from object
}

// Move assignment operator
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
  if (this != &other) {
    if (mRendererId != 0) {
      glDeleteBuffers(1, &mRendererId);
    }
    mRendererId = other.mRendererId;
    mCount = other.mCount;
    other.mRendererId = 0; // Invalidate the moved-from object
  }
  return *this;
}

void IndexBuffer::bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
}

void IndexBuffer::unbind() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
