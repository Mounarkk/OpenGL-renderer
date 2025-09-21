#pragma once
#include "../gl/Texture.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"
#include <memory>

#include "../gl/Shader.h"
#include <fwd.hpp>

class Skybox {
public:
  Skybox();
  void render(const glm::mat4 &projMat, const glm::mat4 &viewMat) const;
  void clean();
  ~Skybox();

private:
  unsigned int mCubeMapTexID;
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::unique_ptr<Shader> mSkyboxShader;
};
