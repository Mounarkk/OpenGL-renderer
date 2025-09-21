#include "VertexArray.h"

#include "GLFW/glfw3.h"

#include <iostream>

VertexArray::VertexArray() {
  glGenVertexArrays(1, &mRendererId); // Generate a new VAO ID from OpenGL
}

VertexArray::~VertexArray() { clean(); }

void VertexArray::clean() {
  if (mRendererId != 0 && glfwGetCurrentContext()) {
    glDeleteVertexArrays(1, &mRendererId); // Delete the VAO from GPU memory
    mRendererId = 0; // Mark as invalid to prevent double-deletion
  }
}

// Move constructor
VertexArray::VertexArray(VertexArray &&other) noexcept
    : mRendererId(other.mRendererId) {
  other.mRendererId = 0; // Invalidate the moved-from object
}

// Move assignment operator
VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
  if (this != &other) {
    if (mRendererId != 0) {
      glDeleteVertexArrays(1, &mRendererId); // Clean up current VAO
    }
    mRendererId = other.mRendererId; // Transfer ownership
    other.mRendererId = 0;           // Invalidate the moved-from object
  }
  return *this;
}

void VertexArray::addBuffer(const VertexBuffer &vb,
                            const VertexBufferLayout &layout) const {
  bind();    // Bind this VAO to capture the following state
  vb.Bind(); // Bind the vertex buffer containing the actual data

  const auto &elements = layout.getElements();
  unsigned int offset = 0;

  // Configure each vertex attribute according to the layout
  for (unsigned int i = 0; i < elements.size(); ++i) {
    const auto &[type, count, normalized] = elements[i];
    glEnableVertexAttribArray(i); // Enable this attribute index for rendering

    // Define how to interpret the vertex data for this attribute
    glVertexAttribPointer(
        i,                         // attribute index
        static_cast<GLint>(count), // number of components (1-4)
        type,                      // data type (float, int, etc.)
        normalized ? GL_TRUE
                   : GL_FALSE, // normalize integers to [0,1] or [-1,1]
        static_cast<GLsizei>(layout.getStride()), // bytes between vertices
        reinterpret_cast<const void *>(
            offset)); // offset to this attribute in vertex

    offset += count *
              VertexBufferElement::getTypeSize(type); // Move to next attribute
  }
}

void VertexArray::bind() const {
  glBindVertexArray(
      mRendererId); // Make this VAO active for subsequent operations
}

void VertexArray::unbind() {
  glBindVertexArray(0); // Unbind any VAO (bind to default state)
}
