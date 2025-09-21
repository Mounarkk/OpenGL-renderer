#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

/**
 * OpenGL Vertex Array Object (VAO) wrapper for managing vertex attribute state.
 *
 * The VertexArray class encapsulates an OpenGL VAO, which stores the
 * configuration of vertex attributes for efficient rendering. It manages the
 * binding of vertex buffers and their associated layouts, allowing for clean
 * separation between vertex data and attribute configuration. This class
 * follows RAII principles with proper move semantics and automatic resource
 * cleanup.
 *
 * Key features:
 * - Automatic VAO creation and cleanup
 * - Move-only semantics to prevent accidental copying
 * - Simple interface for binding vertex buffers with layouts
 * - Efficient vertex attribute state management
 */
class VertexArray {
public:
  /**
   * Constructs a new VAO and generates the OpenGL object.
   */
  VertexArray();

  /**
   * Destructor that automatically cleans up the OpenGL VAO.
   */
  ~VertexArray();

  /**
   * Manually deletes the OpenGL VAO.
   * Called automatically by destructor.
   */
  void clean();

  // Move-only semantics - VAOs should not be copied
  VertexArray(const VertexArray &) = delete;
  VertexArray &operator=(const VertexArray &) = delete;

  /**
   * Move constructor for transferring VAO ownership.
   * @param other VertexArray to move from
   */
  VertexArray(VertexArray &&other) noexcept;

  /**
   * Move assignment operator for transferring VAO ownership.
   * @param other VertexArray to move from
   * @return Reference to this object
   */
  VertexArray &operator=(VertexArray &&other) noexcept;

  /**
   * Associates a vertex buffer with this VAO using the specified layout.
   * Configures vertex attributes according to the layout specification.
   * @param vb Vertex buffer containing the vertex data
   * @param layout Layout specification describing vertex attribute format
   */
  void addBuffer(const VertexBuffer &vb,
                 const VertexBufferLayout &layout) const;

  /**
   * Binds this VAO for rendering operations.
   * All subsequent draw calls will use this VAO's vertex attribute
   * configuration.
   */
  void bind() const;

  /**
   * Unbinds any currently bound VAO.
   * Static method that can be called without a VAO instance.
   */
  static void unbind();

private:
  unsigned int mRendererId = 0;
};