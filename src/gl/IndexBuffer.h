#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "../Renderer.h"

class IndexBuffer {
public:
  IndexBuffer(const unsigned int *data, unsigned int count);
  ~IndexBuffer();

  void bind() const;
  static void unbind();

  [[nodiscard]] inline unsigned int getCount() const { return mCount; }

private:
  unsigned int mRendererId;
  unsigned int mCount;
};

#endif // INDEXBUFFER_H
