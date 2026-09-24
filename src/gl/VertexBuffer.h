#pragma once
#include <glad/glad.h>

/// Immutable GL_ARRAY_BUFFER holding interleaved vertex data. Move-only.
class VertexBuffer {
public:
  /**
   * Uploads the data with GL_STATIC_DRAW.
   * @param data Pointer to the vertex data
   * @param size Size of the data in bytes
   */
  VertexBuffer(const void *data, GLsizeiptr size);
  ~VertexBuffer();

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;
  VertexBuffer(VertexBuffer &&other) noexcept;
  VertexBuffer &operator=(VertexBuffer &&other) noexcept;

  /// Deletes the GL buffer. Safe to call several times.
  void clean();

  void bind() const;
  static void unbind();

private:
  GLuint mRendererId = 0;
};
