#include "Cube.h"

TexturedCube::TexturedCube(const glm::vec3 &position,
                           const std::string &texturePath)
    : Cube(position), mTexture(Texture(texturePath, GL_RGB)) {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO.addBuffer(this->mVBO, layout);
}

void TexturedCube::draw(const Shader &shader, const Camera &camera) const {
  shader.use();
  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  shader.setInt("texture1", 0);
  this->mTexture.bind(0);
  this->mVBO.bind();
  this->mVAO.bind();

  // pass projection matrix to shader (note that in this case it could change
  // every frame)
  const glm::mat4 projection = glm::perspective(
      glm::radians(camera.mZoom),
      static_cast<float>(800) / static_cast<float>(600), 0.1f, 100.0f);
  shader.setMat4("projection", projection);

  // camera/view transformation
  const glm::mat4 view = camera.getViewMatrix();
  shader.setMat4("view", view);

  // calculate the model matrix
  auto model = glm::mat4(
      1.0f); // make sure to initialize matrix to identity matrix first
  model = glm::translate(model, this->mPosition);
  shader.setMat4("model", model);

  // performs the draw call
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glGetError();
}

ColoredCube::ColoredCube(const glm::vec3 &position, const glm::vec3 color)
    : Cube(position), mColor(color) {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO.addBuffer(this->mVBO, layout);
}

void ColoredCube::draw(const Shader &shader, const Camera &camera) const {
  shader.use();
  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  shader.setVec3("objectColor", mColor.x, mColor.y, mColor.z);
  shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
  this->mVBO.bind();
  this->mVAO.bind();

  // pass projection matrix to shader (note that in this case it could change
  // every frame)
  const glm::mat4 projection = glm::perspective(
      glm::radians(camera.mZoom),
      static_cast<float>(800) / static_cast<float>(600), 0.1f, 100.0f);
  shader.setMat4("projection", projection);

  // camera/view transformation
  const glm::mat4 view = camera.getViewMatrix();
  shader.setMat4("view", view);

  // calculate the model matrix
  auto model = glm::mat4(
      1.0f); // make sure to initialize matrix to identity matrix first
  model = glm::translate(model, this->mPosition);
  shader.setMat4("model", model);

  // performs the draw call
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glGetError();
}
