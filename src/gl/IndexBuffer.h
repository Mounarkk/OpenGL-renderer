#pragma once
#include <glad/glad.h>

/**
 * OpenGL Element Buffer Object (EBO/IBO) wrapper for storing vertex indices.
 * 
 * The IndexBuffer class encapsulates an OpenGL EBO, which stores indices that
 * reference vertices in a vertex buffer. This enables efficient rendering by
 * reusing vertices instead of duplicating them, significantly reducing memory
 * usage for complex meshes. The class follows RAII principles with move
 * semantics for safe resource management.
 * 
 * Key features:
 * - Automatic buffer creation and cleanup
 * - Move-only semantics to prevent resource duplication
 * - Index count tracking for draw calls
 * - Efficient GPU memory management
 * 
 * Usage example:
 * ```cpp
 * // Indices for a quad (2 triangles sharing vertices)
 * unsigned int indices[] = {
 *     0, 1, 2,  // first triangle
 *     2, 3, 0   // second triangle
 * };
 * 
 * IndexBuffer ib(indices, 6);  // 6 indices total
 * ib.bind();
 * glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, 0);
 * ```
 */
class IndexBuffer {
public:
  /**
   * Creates an index buffer and uploads index data to GPU memory.
   * @param data Pointer to array of unsigned integers representing vertex indices
   * @param count Number of indices in the array
   */
  IndexBuffer(const unsigned int* data, unsigned int count);
  
  /**
   * Destructor that automatically cleans up the OpenGL buffer.
   */
  ~IndexBuffer();
  
  /**
   * Manually deletes the OpenGL buffer object.
   * Called automatically by destructor.
   */
  void clean();

  // Move-only semantics - index buffers should not be copied
  IndexBuffer(const IndexBuffer&) = delete;
  IndexBuffer& operator=(const IndexBuffer&) = delete;

  /**
   * Move constructor for transferring buffer ownership.
   * @param other IndexBuffer to move from
   */
  IndexBuffer(IndexBuffer&& other) noexcept;
  
  /**
   * Move assignment operator for transferring buffer ownership.
   * @param other IndexBuffer to move from
   * @return Reference to this object
   */
  IndexBuffer& operator=(IndexBuffer&& other) noexcept;

  /**
   * Binds this index buffer for rendering operations.
   * Makes this buffer the active GL_ELEMENT_ARRAY_BUFFER target.
   */
  void bind() const;
  
  /**
   * Unbinds any currently bound index buffer.
   * Static method that can be called without a buffer instance.
   */
  static void unbind();

  /**
   * Gets the number of indices stored in this buffer.
   * Use this value for glDrawElements() calls.
   * @return Number of indices in the buffer
   */
  [[nodiscard]] inline unsigned int getCount() const { return mCount; }

private:
  unsigned int mRendererId = 0;
  unsigned int mCount = 0;
};
