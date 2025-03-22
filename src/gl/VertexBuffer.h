#pragma once
#include <glad/glad.h>

class VertexBuffer {
public:
  VertexBuffer(const void* data, unsigned int size);
  ~VertexBuffer();

  // Disable copying
  VertexBuffer(const VertexBuffer&) = delete;
  VertexBuffer& operator=(const VertexBuffer&) = delete;

  // Enable moving
  VertexBuffer(VertexBuffer&& other) noexcept;
  VertexBuffer& operator=(VertexBuffer&& other) noexcept;

  void Bind() const;
  static void Unbind();

private:
  unsigned int mRendererId = 0;
};