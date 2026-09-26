#pragma once
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

/**
 * Vertex array object: which buffers feed the vertex shader and how their
 * bytes map to attributes. Move-only.
 *
 * Configured with direct state access, so nothing needs to be bound while
 * building it and no other VAO can be modified by accident.
 */
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

  /// Uses the buffer as vertex source, one attribute per layout element
  /// starting at location 0.
  void setVertexBuffer(const VertexBuffer &vb,
                       const VertexBufferLayout &layout) const;

  /// Uses the buffer as index source for glDrawElements.
  void setIndexBuffer(const IndexBuffer &ib) const;

  void bind() const;
  static void unbind();

private:
  GLuint mRendererId = 0;
};
