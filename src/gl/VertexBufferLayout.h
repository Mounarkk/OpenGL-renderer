#pragma once
#include <glad/glad.h>

#include <vector>

/// One vertex attribute: component type and count (a vec3 is GL_FLOAT x 3).
struct VertexBufferElement {
  GLenum type;
  GLint count;
  bool normalized;

  static GLsizei getTypeSize(const GLenum type) {
    switch (type) {
    case GL_FLOAT:
    case GL_UNSIGNED_INT:
    case GL_INT:
      return 4;
    case GL_UNSIGNED_BYTE:
      return 1;
    default:
      return 0;
    }
  }
};

/**
 * Describes interleaved vertex attributes, in shader location order.
 *
 * ```cpp
 * VertexBufferLayout layout;
 * layout.push(GL_FLOAT, 3); // location 0: position
 * layout.push(GL_FLOAT, 2); // location 1: uv
 * vao.setVertexBuffer(vbo, layout);
 * ```
 */
class VertexBufferLayout {
public:
  void push(const GLenum type, const GLint count,
            const bool normalized = false) {
    mElements.push_back({type, count, normalized});
    mStride += VertexBufferElement::getTypeSize(type) * count;
  }

  [[nodiscard]] const std::vector<VertexBufferElement> &getElements() const {
    return mElements;
  }

  /// Size in bytes of one vertex.
  [[nodiscard]] GLsizei getStride() const { return mStride; }

private:
  std::vector<VertexBufferElement> mElements;
  GLsizei mStride = 0;
};
