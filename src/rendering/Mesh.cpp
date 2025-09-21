#include "Mesh.h"

#include "GLFW/glfw3.h"

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<unsigned int> &indices,
           const std::shared_ptr<Material> &material)
    : mMaterial(material) {
  // Create buffers
  mVBO = std::make_unique<VertexBuffer>(vertices.data(),
                                        vertices.size() * sizeof(Vertex));
  mIBO = std::make_unique<IndexBuffer>(indices.data(), indices.size());

  // TODO: basic layout (pos, norm, texcoords)
  // Define vertex layout
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);

  // Create VAO
  mVAO = std::make_unique<VertexArray>();
  mVAO->addBuffer(*mVBO, layout);

  mDestroyed = false;
}

Mesh::~Mesh() {
  if (!mDestroyed && glfwGetCurrentContext()) {
    clean();
  }
}


void Mesh::draw() const {
  //mMaterial->bind();
  mVAO->bind();
  mVBO->Bind();
  mIBO->bind();
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIBO->getCount()),
                 GL_UNSIGNED_INT, nullptr);
}

void Mesh::clean() {
  // Only cleans VAO, VBO and IBO; textures and shaders will be cleaned afterward
  mVAO->clean();
  mVBO->clean();
  mIBO->clean();

  mDestroyed = true;
}
