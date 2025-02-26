#include "IndexBuffer.h"

#include <iostream>

IndexBuffer::IndexBuffer(const unsigned int *data, const unsigned int count)
    : mCount(count) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data,
               GL_STATIC_DRAW);
  std::cout << "IBO created with ID: " << mRendererId << std::endl;
}

IndexBuffer::~IndexBuffer() {
  glDeleteBuffers(1, &mRendererId);
  std::cout << "Deleting IBO ID: " << mRendererId << std::endl;
}

void IndexBuffer::bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererId);
}

void IndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
