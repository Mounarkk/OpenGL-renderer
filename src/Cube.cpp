#include "Cube.h"

Cube::Cube(const glm::vec3 &position, const std::string& texturePath) : mPosition(position), mTexture(Texture(texturePath, GL_RGB)),
mVAO(VertexArray()), mVBO(VertexBuffer(Cube::vertices, sizeof(vertices))) {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO.addBuffer(this->mVBO, layout);
}


void Cube::draw(const Shader &shader) const {
  // calculate the model matrix
  auto model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  model = glm::translate(model, this->mPosition);

  // set the appropriate uniform
  shader.setMat4("model", model);

  // performs the draw call
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

