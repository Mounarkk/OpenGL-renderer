#pragma once
#include <glad/glad.h>

/// Immutable GPU buffer of 32-bit indices. Move-only.
class IndexBuffer {
public:
  /// @param data Indices to upload
  /// @param count Number of indices
  IndexBuffer(const unsigned int *data, GLsizei count);
  ~IndexBuffer();

  IndexBuffer(const IndexBuffer &) = delete;
  IndexBuffer &operator=(const IndexBuffer &) = delete;
  IndexBuffer(IndexBuffer &&other) noexcept;
  IndexBuffer &operator=(IndexBuffer &&other) noexcept;

  /// Deletes the GL buffer. Safe to call several times.
  void clean();

  [[nodiscard]] GLuint getID() const { return mRendererId; }

  /// Number of indices, to pass to glDrawElements.
  [[nodiscard]] GLsizei getCount() const { return mCount; }

private:
  GLuint mRendererId = 0;
  GLsizei mCount = 0;
};
