#include "Skybox.h"
#include "../core/Config.h"
#include "../core/Logger.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <array>

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

Skybox::Skybox(const std::string &directory) {
  // Order of GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
  const std::array<std::string, 6> faces = {"right.jpg", "left.jpg",
                                            "top.jpg",   "bottom.jpg",
                                            "front.jpg", "back.jpg"};

  glGenTextures(1, &mCubeMapTexID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMapTexID);

  // Cube map faces follow the image convention (top row first)
  stbi_set_flip_vertically_on_load(false);
  for (size_t i = 0; i < faces.size(); ++i) {
    const std::string path = directory + faces[i];
    int width, height, channels;
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &channels, 3);
    if (!data) {
      Logger::get()->error("Failed to load skybox face {}", path);
      continue;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0,
                 GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  mVAO = std::make_unique<VertexArray>();
  mVBO = std::make_unique<VertexBuffer>(kCubeVertices, sizeof(kCubeVertices));
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 3);
  mVAO->addBuffer(*mVBO, layout);
  VertexArray::unbind();

  const std::string shaderDir = Config::getShaderPath();
  mShader = std::make_unique<Shader>(shaderDir + "skybox.vert",
                                     shaderDir + "skybox.frag");
}

Skybox::~Skybox() {
  if (mCubeMapTexID != 0 && glfwGetCurrentContext())
    glDeleteTextures(1, &mCubeMapTexID);
}

void Skybox::render(const glm::mat4 &projMat, const glm::mat4 &viewMat) const {
  glDepthFunc(GL_LEQUAL);

  mShader->use();
  // Dropping the translation keeps the sky infinitely far away
  mShader->setMat4("projection", projMat);
  mShader->setMat4("view", glm::mat4(glm::mat3(viewMat)));
  mShader->setInt("skybox", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMapTexID);
  mVAO->bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  VertexArray::unbind();

  glDepthFunc(GL_LESS);
}
