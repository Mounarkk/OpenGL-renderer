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

  void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const;
  void Bind() const;
  static void Unbind();

private:
  unsigned int mRendererId = 0;
};