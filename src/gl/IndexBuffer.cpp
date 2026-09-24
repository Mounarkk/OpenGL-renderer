#include "IndexBuffer.h"

#include <GLFW/glfw3.h>

IndexBuffer::IndexBuffer(const unsigned int *data, const GLsizei count)
    : mCount(count) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data,
               GL_STATIC_DRAW);
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

void IndexBuffer::bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
}

void IndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
