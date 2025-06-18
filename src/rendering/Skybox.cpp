#include "Skybox.h"

#include "GLFW/glfw3.h"

#include <stb_image.h>

#include <memory>

Skybox::Skybox() {
  // Loading the appropriate textures and their parameters
  const std::vector<std::string> faces = {
    "right.jpg",
    "left.jpg",
    "top.jpg",
    "bottom.jpg",
    "front.jpg",
    "back.jpg"
  };

  mCubeMapTexID = 0;
  glGenTextures(1, &mCubeMapTexID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMapTexID);

  // Load each face texture
  int width, height, channels;
  auto resourcePath = "../res/skyboxes/arctic/";
  for (int i = 0; i < faces.size(); i++) {
    unsigned char* data = stbi_load( (resourcePath + faces[i]).c_str(), &width, &height, &channels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      Logger::get()->error("Failed to load texture: {}", resourcePath + faces[i]);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // Set up the cube data
  float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  mVBO = std::make_unique<VertexBuffer>(skyboxVertices, sizeof(skyboxVertices));

  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 3);

  mVAO = std::make_unique<VertexArray>();
  mVAO->addBuffer(*mVBO, layout);

  // Load the associated shader
  mSkyboxShader = std::make_unique<Shader>("../res/shaders/skybox.vert", "../res/shaders/skybox.frag");
}

void Skybox::render(const glm::mat4 &projMat, const glm::mat4 &viewMat) const {
  glDepthFunc(GL_LEQUAL);

  mSkyboxShader->use();
  mVAO->bind();
  mVBO->Bind();

  // Set uniforms
  mSkyboxShader->setMat4("projection", projMat);
  mSkyboxShader->setMat4("view", glm::mat4(glm::mat3(viewMat)));
  mSkyboxShader->setInt("skybox", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMapTexID);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glDepthFunc(GL_LESS);
}

void Skybox::clean() {
  mVBO->clean();
  mVAO->clean();
  mSkyboxShader->clean();

  if (mCubeMapTexID != 0 && glfwGetCurrentContext()) {
    Logger::get()->info("Deleted texture ID: {}", mCubeMapTexID);
    glDeleteTextures(1, &mCubeMapTexID);
    mCubeMapTexID = 0;
  }
}

Skybox::~Skybox() {
  clean();
}





