#include "VertexBuffer.h"

#include <iostream>

VertexBuffer::VertexBuffer(const void *data, const unsigned int size) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  std::cout << "VBO created with ID: " << mRendererId << std::endl;
}

VertexBuffer::~VertexBuffer() {

  std::cout << "Deleting VBO ID: " << mRendererId << std::endl;
}

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, mRendererId); }

void VertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
