#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  // Disable copying
  VertexArray(const VertexArray&) = delete;
  VertexArray& operator=(const VertexArray&) = delete;

  // Enable moving
  VertexArray(VertexArray&& other) noexcept;
  VertexArray& operator=(VertexArray&& other) noexcept;

  void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const;
  void bind() const;
  static void unbind();

private:
  unsigned int mRendererId = 0;
};