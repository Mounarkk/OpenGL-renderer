#include "Skybox.h"
#include "../core/Config.h"
#include "../core/Logger.h"
#include "../resource/ResourceManager.h"
#include "ShaderInterface.h"

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

  // Cube map faces follow the image convention (top row first)
  stbi_set_flip_vertically_on_load(false);
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mCubeMapTexID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (size_t face = 0; face < faces.size(); ++face) {
    const std::string path = directory + faces[face];
    int width, height, channels;
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &channels, 3);
    if (!data) {
      Logger::get()->error("Failed to load skybox face {}", path);
      continue;
    }
    // Storage is allocated with the size of the first face found
    if (!mAllocated) {
      glTextureStorage2D(mCubeMapTexID, 1, GL_SRGB8, width, height);
      mAllocated = true;
    }
    // With DSA, cube map faces are the layers 0 to 5 of the texture
    glTextureSubImage3D(mCubeMapTexID, 0, 0, 0, static_cast<GLint>(face), width,
                        height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  glTextureParameteri(mCubeMapTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(mCubeMapTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(mCubeMapTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(mCubeMapTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(mCubeMapTexID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  mVBO = std::make_unique<VertexBuffer>(kCubeVertices, sizeof(kCubeVertices));
  mVAO = std::make_unique<VertexArray>();
  VertexBufferLayout layout;
  layout.push(GL_FLOAT, 3);
  mVAO->setVertexBuffer(*mVBO, layout);

  const std::string shaderDir = Config::getShaderPath();
  mShader = ResourceManager::loadShader(shaderDir + "skybox.vert",
                                        shaderDir + "skybox.frag");
}

Skybox::~Skybox() {
  if (mCubeMapTexID != 0 && glfwGetCurrentContext())
    glDeleteTextures(1, &mCubeMapTexID);
}

void Skybox::render() const {
  // Drawn at the far plane: only fills pixels no geometry wrote to
  glDepthFunc(GL_LEQUAL);

  mShader->use();
  glBindTextureUnit(ShaderInterface::kSkyboxUnit, mCubeMapTexID);
  mVAO->bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  VertexArray::unbind();

  glDepthFunc(GL_LESS);
}
