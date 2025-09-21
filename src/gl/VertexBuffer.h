#pragma once
#include <glad/glad.h>

/**
 * OpenGL Vertex Buffer Object (VBO) wrapper for storing vertex data on the GPU.
 * 
 * The VertexBuffer class encapsulates an OpenGL VBO, which stores vertex data
 * (positions, normals, texture coordinates, etc.) in GPU memory for efficient
 * rendering. It handles buffer creation, data upload, and binding operations
 * while following RAII principles with move semantics to prevent resource leaks.
 * 
 * Key features:
 * - Automatic buffer creation and cleanup
 * - Move-only semantics to prevent accidental copying
 * - Efficient GPU memory management
 * - Simple binding interface for rendering operations
 * 
 * Usage example:
 * ```cpp
 * // Vertex data (position + texture coordinates)
 * float vertices[] = {
 *     -0.5f, -0.5f, 0.0f, 0.0f,  // bottom-left
 *      0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
 *      0.0f,  0.5f, 0.5f, 1.0f   // top-center
 * };
 * 
 * VertexBuffer vb(vertices, sizeof(vertices));
 * vb.Bind();  // Ready for rendering
 * ```
 */
class VertexBuffer {
public:
  /**
   * Creates a vertex buffer and uploads data to GPU memory.
   * @param data Pointer to vertex data to upload
   * @param size Size of the data in bytes
   */
  VertexBuffer(const void* data, unsigned int size);
  
  /**
   * Destructor that automatically cleans up the OpenGL buffer.
   */
  ~VertexBuffer();
  
  /**
   * Manually deletes the OpenGL buffer object.
   * Called automatically by destructor.
   */
  void clean();

  // Move-only semantics - vertex buffers should not be copied
  VertexBuffer(const VertexBuffer&) = delete;
  VertexBuffer& operator=(const VertexBuffer&) = delete;

  /**
   * Move constructor for transferring buffer ownership.
   * @param other VertexBuffer to move from
   */
  VertexBuffer(VertexBuffer&& other) noexcept;
  
  /**
   * Move assignment operator for transferring buffer ownership.
   * @param other VertexBuffer to move from
   * @return Reference to this object
   */
  VertexBuffer& operator=(VertexBuffer&& other) noexcept;

  /**
   * Binds this vertex buffer for rendering operations.
   * Makes this buffer the active GL_ARRAY_BUFFER target.
   */
  void Bind() const;
  
  /**
   * Unbinds any currently bound vertex buffer.
   * Static method that can be called without a buffer instance.
   */
  static void Unbind();

private:
  unsigned int mRendererId = 0;
};