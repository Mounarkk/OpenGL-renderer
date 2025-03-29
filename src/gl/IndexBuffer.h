#pragma once
#include <glad/glad.h>

class IndexBuffer {
public:
  IndexBuffer(const unsigned int* data, unsigned int count);
  ~IndexBuffer();

  // Disable copying
  IndexBuffer(const IndexBuffer&) = delete;
  IndexBuffer& operator=(const IndexBuffer&) = delete;

  // Enable moving
  IndexBuffer(IndexBuffer&& other) noexcept;
  IndexBuffer& operator=(IndexBuffer&& other) noexcept;

  void bind() const;
  static void unbind();

  [[nodiscard]] inline unsigned int getCount() const { return mCount; }

private:
  unsigned int mRendererId = 0;
  unsigned int mCount = 0;
};
