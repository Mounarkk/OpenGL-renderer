#include "Cube.h"

Cube::Cube(const glm::vec3 &position, const std::string& texturePath) : mPosition(position), mVAO(VertexArray()),
                                                                        mVBO(VertexBuffer(this->vertices, sizeof(this->vertices))), mTexture(Texture(texturePath, GL_RGB)){

  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO.addBuffer(this->mVBO, layout);
}


void Cube::draw(const Shader &shader, const Camera &camera) const {
  this->mTexture.bind(0);

  shader.use();
  this->mVBO.bind();
  this->mVAO.bind();

  // pass projection matrix to shader (note that in this case it could change every frame)
  glm::mat4 projection = glm::perspective(glm::radians(camera.mZoom), static_cast<float>(800) / static_cast<float>(600), 0.1f, 100.0f);
  shader.setMat4("projection", projection);

  // camera/view transformation
  glm::mat4 view = camera.getViewMatrix();
  shader.setMat4("view", view);

  // calculate the model matrix
  auto model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  model = glm::translate(model, this->mPosition);
  shader.setMat4("model", model);

  // performs the draw call
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

