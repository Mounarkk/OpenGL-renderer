#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <glad/glad.h>
#include <memory>

class FrameBuffer {
public:
  FrameBuffer(int width, int height);
  ~FrameBuffer();
  void clean();

  // Disable copying
  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;

  // Enable moving
  FrameBuffer(FrameBuffer &&other) noexcept;
  FrameBuffer& operator=(FrameBuffer&& other) noexcept;

  void bind() const;
  static void unbind();
  static void unbindOther(unsigned int id);

  unsigned int mTextureID;
  unsigned int mRboID;
private:
  unsigned int mRendererID = 0;
  int mWidth, mHeight;
};