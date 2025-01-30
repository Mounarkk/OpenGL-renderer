#include "Cube.h"

TexturedCube::TexturedCube(const glm::vec3 &position,
                           const std::string &diffusePath,
                           const int diffuseFormat,
                           const std::string &specularPath,
                           const int specularFormat,
                           const std::string &emissionPath,
                           const int emissionFormat)
    : Cube(position), mVBO(VertexBuffer(vertices, sizeof(vertices))),
      mDiffuseTexture(Texture(diffusePath, diffuseFormat)),
      mSpecularTexture(Texture(specularPath, specularFormat)),
      mEmissionTexture(Texture(emissionPath, emissionFormat)) {
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 3);
  layout.Push(GL_FLOAT, 2);
  this->mVAO.addBuffer(this->mVBO, layout);
}

void TexturedCube::draw(const Shader &shader, const Camera &camera) const {
  shader.use();
  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  shader.setInt("material.diffuse", 0);
  shader.setInt("material.specular", 1);
  shader.setInt("material.emission", 2);
  shader.setFloat("material.shininess", 64.0f);

  shader.setVec3("light.position", 4.0f, 2.0f, -5.7f);
  shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
  shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
  shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("light.constant", 1.0f);
  shader.setFloat("light.linear", 0.045f);
  shader.setFloat("light.quadratic", 0.0075);

  this->mDiffuseTexture.bind(0);
  this->mSpecularTexture.bind(1);
  this->mEmissionTexture.bind(2);
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
    : Cube(position),
      mVBO(VertexBuffer(coloredCubeVertices, sizeof(coloredCubeVertices))),
      mColor(color) {
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
