#pragma once
#include <glad/glad.h>

/// Immutable GPU buffer holding interleaved vertex data. Move-only.
class VertexBuffer {
public:
  /// @param data Vertex data to upload
  /// @param size Size of the data in bytes
  VertexBuffer(const void *data, GLsizeiptr size);
  ~VertexBuffer();

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;
  VertexBuffer(VertexBuffer &&other) noexcept;
  VertexBuffer &operator=(VertexBuffer &&other) noexcept;

  /// Deletes the GL buffer. Safe to call several times.
  void clean();

  [[nodiscard]] GLuint getID() const { return mRendererId; }

private:
  GLuint mRendererId = 0;
};
