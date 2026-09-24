#include "VertexArray.h"

#include <GLFW/glfw3.h>

#include <cstdint>

VertexArray::VertexArray() { glGenVertexArrays(1, &mRendererId); }

VertexArray::~VertexArray() { clean(); }

void VertexArray::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteVertexArrays(1, &mRendererId);
    mRendererId = 0;
  }
}

VertexArray::VertexArray(VertexArray &&other) noexcept
    : mRendererId(other.mRendererId) {
  other.mRendererId = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
  if (this != &other) {
    clean();
    mRendererId = other.mRendererId;
    other.mRendererId = 0;
  }
  return *this;
}

void VertexArray::addBuffer(const VertexBuffer &vb,
                            const VertexBufferLayout &layout) const {
  bind();
  vb.bind();

  const auto &elements = layout.getElements();
  std::uintptr_t offset = 0;
  for (GLuint i = 0; i < elements.size(); ++i) {
    const auto &[type, count, normalized] = elements[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, count, type, normalized ? GL_TRUE : GL_FALSE,
                          layout.getStride(),
                          reinterpret_cast<const void *>(offset));
    offset += static_cast<std::uintptr_t>(
        count * VertexBufferElement::getTypeSize(type));
  }
}

void VertexArray::bind() const { glBindVertexArray(mRendererId); }

void VertexArray::unbind() { glBindVertexArray(0); }
