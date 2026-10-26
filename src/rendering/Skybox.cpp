#include "Skybox.h"
#include "../core/Config.h"
#include "../resource/ResourceManager.h"
#include "ShaderInterface.h"

namespace {
// Unit cube, 36 vertices, viewed from the inside
constexpr float kCubeVertices[] = {
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
} // namespace

Skybox::Skybox() {
  mVBO = std::make_unique<VertexBuffer>(kCubeVertices, sizeof(kCubeVertices));
  mVAO = std::make_unique<VertexArray>();
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 3);
  mVAO->setVertexBuffer(*mVBO, layout);

  const std::string shaderDir = Config::getShaderPath();
  mShader = ResourceManager::loadShader(shaderDir + "skybox.vert",
                                        shaderDir + "skybox.frag");
}

void Skybox::render(const GLuint cubeMap, const float level) const {
  // Drawn at the far plane: only fills pixels no geometry wrote to
  glDepthFunc(GL_LEQUAL);

  mShader->use();
  mShader->setFloat("uLevel", level);
  glBindTextureUnit(ShaderInterface::kSkyboxUnit, cubeMap);
  mVAO->bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  VertexArray::unbind();

  glDepthFunc(GL_LESS);
}
