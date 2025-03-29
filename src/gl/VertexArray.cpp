#include "VertexArray.h"
#include <iostream>

VertexArray::VertexArray() {
  glGenVertexArrays(1, &mRendererId);
}

VertexArray::~VertexArray() {
  if (mRendererId != 0) {
    glDeleteVertexArrays(1, &mRendererId);
  }
}

// Move constructor
VertexArray::VertexArray(VertexArray&& other) noexcept
    : mRendererId(other.mRendererId) {
  other.mRendererId = 0; // Invalidate the moved-from object
}

// Move assignment operator
VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
  if (this != &other) {
    if (mRendererId != 0) {
      glDeleteVertexArrays(1, &mRendererId);
    }
    mRendererId = other.mRendererId;
    other.mRendererId = 0; // Invalidate the moved-from object
  }
  return *this;
}

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const {
  bind();
  vb.Bind();

  const auto& elements = layout.getElements();
  unsigned int offset = 0;

  for (unsigned int i = 0; i < elements.size(); ++i) {
    const auto& [type, count, normalized] = elements[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, static_cast<GLint>(count), type, normalized ? GL_TRUE : GL_FALSE,
                          static_cast<GLsizei>(layout.getStride()), reinterpret_cast<const void*>(offset));
    offset += count * VertexBufferElement::getTypeSize(type);
  }
}

void VertexArray::bind() const {
  glBindVertexArray(mRendererId);
}

void VertexArray::unbind() {
  glBindVertexArray(0);
}
