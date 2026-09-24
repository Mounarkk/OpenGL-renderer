#pragma once
#include "../gl/Shader.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"

#include <memory>
#include <string>

/**
 * Cube map drawn behind everything else.
 *
 * Expects right, left, top, bottom, front and back .jpg faces in the given
 * directory. It is drawn last with GL_LEQUAL so it only fills pixels that no
 * geometry wrote to.
 */
class Skybox {
public:
  explicit Skybox(const std::string &directory);
  ~Skybox();

  Skybox(const Skybox &) = delete;
  Skybox &operator=(const Skybox &) = delete;

  void render(const glm::mat4 &projMat, const glm::mat4 &viewMat) const;

private:
  GLuint mCubeMapTexID = 0;
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::unique_ptr<Shader> mShader;
};
