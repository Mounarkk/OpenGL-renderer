#pragma once
#include <vector>
#include <glad/glad.h>

/**
 * Describes a single vertex attribute element (position, normal, texture coords, etc.).
 * 
 * This structure defines how a single vertex attribute should be interpreted
 * by OpenGL, including its data type, component count, and normalization settings.
 */
struct VertexBufferElement {
  unsigned int type;        // OpenGL data type (GL_FLOAT, GL_UNSIGNED_INT, etc.)
  unsigned int count;       // Number of components (1-4, e.g., vec3 has count=3)
  bool normalized;          // Whether to normalize integer values to [0,1] or [-1,1]

  /**
   * Gets the size in bytes of an OpenGL data type.
   * @param type OpenGL type constant (GL_FLOAT, GL_UNSIGNED_INT, etc.)
   * @return Size in bytes, or 0 if type is unknown
   */
  static unsigned int getTypeSize(const unsigned int type) {
    switch (type) {
    case GL_FLOAT:
      return 4;
    case GL_UNSIGNED_INT:
      return 4;
    default:
      return 0;
    }
  }
};

/**
 * Defines the layout of vertex attributes in a vertex buffer.
 * 
 * The VertexBufferLayout class describes how vertex data is organized in memory,
 * specifying the format of each attribute (position, normal, texture coordinates, etc.).
 * This information is used by VertexArray to configure OpenGL vertex attribute pointers.
 * 
 * Usage example:
 * ```cpp
 * VertexBufferLayout layout;
 * layout.Push(GL_FLOAT, 3);  // Position: 3 floats (x, y, z)
 * layout.Push(GL_FLOAT, 2);  // Texture coords: 2 floats (u, v)
 * layout.Push(GL_FLOAT, 3);  // Normal: 3 floats (nx, ny, nz)
 * 
 * vertexArray.addBuffer(vertexBuffer, layout);
 * ```
 */
class VertexBufferLayout {
public:
  /**
   * Constructs an empty vertex buffer layout.
   */
  VertexBufferLayout() : mStride(0){};

  /**
   * Adds a vertex attribute to the layout.
   * @param type OpenGL data type (GL_FLOAT, GL_UNSIGNED_INT, etc.)
   * @param count Number of components for this attribute (1-4)
   */
  void Push(const unsigned int type, const unsigned int count) {
    VertexBufferElement element = {type, count, GL_FALSE};
    mElements.emplace_back(element);
    mStride += VertexBufferElement::getTypeSize(type) * count;  // Update total vertex size
  }

  /**
   * Gets all vertex attribute elements in this layout.
   * @return Vector of vertex buffer elements describing each attribute
   */
  [[nodiscard]] std::vector<VertexBufferElement> getElements() const {
    return mElements;
  }
  
  /**
   * Gets the stride (total size in bytes) of a single vertex.
   * @return Size in bytes of one complete vertex with all attributes
   */
  [[nodiscard]] unsigned int getStride() const { return mStride; }

private:
  std::vector<VertexBufferElement> mElements;
  unsigned int mStride;
};
