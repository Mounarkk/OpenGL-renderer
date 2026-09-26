#include "VertexArray.h"

#include <GLFW/glfw3.h>

VertexArray::VertexArray() { glCreateVertexArrays(1, &mRendererId); }

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

void VertexArray::setVertexBuffer(const VertexBuffer &vb,
                                  const VertexBufferLayout &layout) const {
  // All attributes come from binding slot 0, interleaved
  constexpr GLuint bindingIndex = 0;
  glVertexArrayVertexBuffer(mRendererId, bindingIndex, vb.getID(), 0,
                            layout.getStride());

  GLuint offset = 0;
  const auto &elements = layout.getElements();
  for (GLuint location = 0; location < elements.size(); ++location) {
    const auto &[type, count, normalized] = elements[location];
    glEnableVertexArrayAttrib(mRendererId, location);
    glVertexArrayAttribFormat(mRendererId, location, count, type,
                              normalized ? GL_TRUE : GL_FALSE, offset);
    glVertexArrayAttribBinding(mRendererId, location, bindingIndex);
    offset +=
        static_cast<GLuint>(count * VertexBufferElement::getTypeSize(type));
  }
}

void VertexArray::setIndexBuffer(const IndexBuffer &ib) const {
  glVertexArrayElementBuffer(mRendererId, ib.getID());
}

void VertexArray::bind() const { glBindVertexArray(mRendererId); }

void VertexArray::unbind() { glBindVertexArray(0); }
