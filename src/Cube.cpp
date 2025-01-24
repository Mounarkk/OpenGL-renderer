#include "Cube.h"

TexturedCube::TexturedCube(const glm::vec3 &position,
                           const std::string &texturePath)
    : Cube(position), mVBO(VertexBuffer(texturedCubeVertices, sizeof(texturedCubeVertices))), mTexture(Texture(texturePath, GL_RGB)) {
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
}

ColoredCube::ColoredCube(const glm::vec3 &position, const glm::vec3 color)
    : Cube(position), mVBO(VertexBuffer(coloredCubeVertices, sizeof(coloredCubeVertices))), mColor(color) {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  this->mVAO.addBuffer(this->mVBO, layout);
}

void ColoredCube::draw(const Shader &shader, const Camera &camera) const {
  shader.use();
  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------

  shader.setVec3("material.ambient", 0.1f, 0.18725f, 0.1745f);
  shader.setVec3("material.diffuse", 0.396f, 0.74151f, 0.69102f);
  shader.setVec3("material.specular", 0.297254f, 0.30829f, 0.306678f);
  shader.setFloat("material.shininess", 0.6 * 128);

  shader.setVec3("light.position", 0.0f, 1.0f, -4.7f);
  shader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
  shader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
  shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

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
}


