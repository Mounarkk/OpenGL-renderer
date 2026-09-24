#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

/// Vertex array object recording the attribute setup of a buffer. Move-only.
class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  VertexArray(const VertexArray &) = delete;
  VertexArray &operator=(const VertexArray &) = delete;
  VertexArray(VertexArray &&other) noexcept;
  VertexArray &operator=(VertexArray &&other) noexcept;

  /// Deletes the GL object. Safe to call several times.
  void clean();

  /// Binds the buffer and enables one attribute per layout element, starting
  /// at location 0.
  void addBuffer(const VertexBuffer &vb,
                 const VertexBufferLayout &layout) const;

  void bind() const;
  static void unbind();

private:
  GLuint mRendererId = 0;
};
