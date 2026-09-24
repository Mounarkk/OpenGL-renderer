#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<unsigned int> &indices) {
  mVAO = std::make_unique<VertexArray>();
  mVAO->bind();

  mVBO = std::make_unique<VertexBuffer>(
      vertices.data(),
      static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)));
  mIBO = std::make_unique<IndexBuffer>(indices.data(),
                                       static_cast<GLsizei>(indices.size()));

  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 3); // position
  layout.push(GL_FLOAT, 3); // normal
  layout.push(GL_FLOAT, 3); // tangent
  layout.push(GL_FLOAT, 3); // bitangent
  layout.push(GL_FLOAT, 2); // texCoords
  mVAO->addBuffer(*mVBO, layout);

  VertexArray::unbind();
}

void Mesh::draw() const {
  mVAO->bind();
  glDrawElements(GL_TRIANGLES, mIBO->getCount(), GL_UNSIGNED_INT, nullptr);
}
