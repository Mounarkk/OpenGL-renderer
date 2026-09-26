#include "Mesh.h"

#include <limits>

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<unsigned int> &indices)
    : mVBO(vertices.data(),
           static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex))),
      mIBO(indices.data(), static_cast<GLsizei>(indices.size())) {
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 3); // position
  layout.push(GL_FLOAT, 3); // normal
  layout.push(GL_FLOAT, 3); // tangent
  layout.push(GL_FLOAT, 3); // bitangent
  layout.push(GL_FLOAT, 2); // texCoords
  mVAO.setVertexBuffer(mVBO, layout);
  mVAO.setIndexBuffer(mIBO);

  mBounds.min = glm::vec3(std::numeric_limits<float>::max());
  mBounds.max = glm::vec3(std::numeric_limits<float>::lowest());
  for (const Vertex &vertex : vertices) {
    mBounds.min = glm::min(mBounds.min, vertex.position);
    mBounds.max = glm::max(mBounds.max, vertex.position);
  }
  if (vertices.empty())
    mBounds = {};
}

void Mesh::draw() const {
  mVAO.bind();
  glDrawElements(GL_TRIANGLES, mIBO.getCount(), GL_UNSIGNED_INT, nullptr);
}
