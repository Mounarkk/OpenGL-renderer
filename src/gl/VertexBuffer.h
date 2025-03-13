#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <glad/glad.h>

class VertexBuffer {
public:
  VertexBuffer(const void *data, unsigned int size);
  ~VertexBuffer();

  void bind() const;
  static void unbind();

private:
  unsigned int mRendererId;
};

#endif // VERTEXBUFFER_H
