#pragma once
#include "../gl/Shader.h"
#include "../gl/VertexArray.h"
#include "../gl/VertexBuffer.h"

#include <memory>

/**
 * Cube map drawn behind everything else, usually the environment map.
 *
 * It is drawn last with GL_LEQUAL so it only fills pixels that no geometry
 * wrote to. The sun is drawn on top of the cube map where the directional
 * light comes from, which makes the shadows easy to check.
 */
class Skybox {
public:
  Skybox();

  /// Draws the sky. Reads the camera and the sun from the frame and light
  /// uniform blocks, which must be bound.
  /// @param level Mip level to read, negative to let the GPU pick it
  void render(GLuint cubeMap, float level = -1.0f) const;

private:
  std::unique_ptr<VertexArray> mVAO;
  std::unique_ptr<VertexBuffer> mVBO;
  std::shared_ptr<Shader> mShader;
};
