#pragma once
#include <glad/glad.h>

/**
 * Buffer backing a GLSL `uniform` block. Move-only.
 *
 * The content is written from the CPU once per frame (or less) and read by
 * every shader that declares the block at the same binding point, so shared
 * data such as the camera or the lights is uploaded once instead of once per
 * program.
 */
class UniformBuffer {
public:
  /// Allocates `size` bytes of uninitialized storage.
  explicit UniformBuffer(GLsizeiptr size);
  ~UniformBuffer();

  UniformBuffer(const UniformBuffer &) = delete;
  UniformBuffer &operator=(const UniformBuffer &) = delete;
  UniformBuffer(UniformBuffer &&other) noexcept;
  UniformBuffer &operator=(UniformBuffer &&other) noexcept;

  /// Deletes the GL buffer. Safe to call several times.
  void clean();

  /// Copies `size` bytes from `data` at `offset` bytes into the buffer.
  void update(const void *data, GLsizeiptr size, GLintptr offset = 0) const;

  /// Makes the buffer visible to the blocks declared with this binding.
  void bindBase(GLuint binding) const;

  [[nodiscard]] GLsizeiptr getSize() const { return mSize; }

private:
  GLuint mRendererId = 0;
  GLsizeiptr mSize = 0;
};
