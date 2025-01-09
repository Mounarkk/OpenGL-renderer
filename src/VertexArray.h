#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Renderer.h"

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  void addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout) const;

  void bind() const;
  static void unbind() ;
private:
  unsigned int mRendererId;
};



#endif //VERTEXARRAY_H
