#pragma once
#include <vector>

struct VertexBufferElement {
  unsigned int type;
  unsigned int count;
  bool normalized;

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

class VertexBufferLayout {
public:
  VertexBufferLayout() : mStride(0){};

  void Push(const unsigned int type, const unsigned int count) {
    VertexBufferElement element = {type, count, GL_FALSE};
    mElements.emplace_back(element);
    mStride += VertexBufferElement::getTypeSize(type) * count;
  }

  [[nodiscard]] std::vector<VertexBufferElement> getElements() const {
    return mElements;
  }
  [[nodiscard]] unsigned int getStride() const { return mStride; }

private:
  std::vector<VertexBufferElement> mElements;
  unsigned int mStride;
};
