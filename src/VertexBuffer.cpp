#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(const void *data, const unsigned int size) {
  glGenBuffers(1, &mRendererId);
  glBindBuffer(GL_ARRAY_BUFFER, mRendererId);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() { glDeleteBuffers(1, &mRendererId); }

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, mRendererId); }

void VertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
