#include "VertexArray.h"

#include <iostream>

VertexArray::VertexArray() {
  glGenVertexArrays(1, &mRendererId);
  std::cout << "VAO created with ID: " << mRendererId << std::endl;
}

VertexArray::~VertexArray() {
  std::cout << "Deleting VAO ID: " << mRendererId << std::endl;

}

void VertexArray::addBuffer(const VertexBuffer &vb,
                            const VertexBufferLayout &layout) const {
  this->bind();
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) std::cout << "OpenGL error after addBuffer and after enablind vertex attrib array: " << err << std::endl;
  vb.bind();
  const auto &elements = layout.getElements();

  unsigned int offset = 0;
  for (unsigned int i = 0; i < elements.size(); ++i) {
    const auto &[type, count, normalized] = elements[i];

    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, static_cast<GLint>(count), type,
                          normalized ? GL_TRUE : GL_FALSE,
                          static_cast<GLsizei>(layout.getStride()),
                          reinterpret_cast<const void *>(offset));

    offset += count * VertexBufferElement::getTypeSize(type);
  }
}

void VertexArray::bind() const { glBindVertexArray(mRendererId); }

void VertexArray::unbind() { glBindVertexArray(0); }
