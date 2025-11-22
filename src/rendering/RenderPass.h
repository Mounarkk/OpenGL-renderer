#pragma once
#include "../gl/FrameBuffer.h"
#include "../gl/Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "Skybox.h"

#include "../scene/Components.h"
#include <memory>

struct RenderCommand {
  Transform transform;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};

class RenderPass {
public:
  virtual ~RenderPass() = default;
  virtual void execute(std::vector<RenderCommand> &s_CommandQueue,
                       const glm::mat4 &projMat, const glm::mat4 &viewMat) = 0;
  virtual void cleanup() = 0;
  virtual void resize(int width, int height) = 0;

protected:
  std::shared_ptr<Shader> mShader;
};

class ShadowMappingPass final : public RenderPass {
public:
  ShadowMappingPass(const std::string &vertexPath,
    const std::string &fragmentPath, int depthMapWidth,
    int depthMapHeight);
  ~ShadowMappingPass() override;

  void execute(std::vector<RenderCommand> &s_CommandQueue,
               const glm::mat4 &projMat, const glm::mat4 &viewMat) override;
  void cleanup() override;
  void resize(int width, int height) override;

  [[nodiscard]] int getTextDepthBufferID() const;
  [[nodiscard]] glm::mat4 getDirLightProjView() const;

private:
  std::unique_ptr<FrameBuffer> mFBO;
  glm::mat4 mDirLightProjView;
};

class ForwardLightingPass final : public RenderPass {
public:
  ForwardLightingPass(const std::string &vertexPath,
                      const std::string &fragmentPath, int width, int height,
                      int dirLightShadowMapBufferID,
                      const glm::mat4 &dirLightProjView);
  ~ForwardLightingPass() override;

  void execute(std::vector<RenderCommand> &s_CommandQueue,
               const glm::mat4 &projMat, const glm::mat4 &viewMat) override;
  void cleanup() override;
  void resize(int width, int height) override;

  [[nodiscard]] int getTextColorBufferID() const;

private:
  std::unique_ptr<FrameBuffer> mFBO;
  std::unique_ptr<Skybox> mSkybox;
  int mDirLightShadowMapBufferID;
  glm::mat4 mDirLightProjView;
};

class PostProcessingPass final : public RenderPass {
public:
  PostProcessingPass(const std::string &vertexPath,
                     const std::string &fragmentPath, int texColorBufferID);
  ~PostProcessingPass() override;

  void execute(std::vector<RenderCommand> &s_CommandQueue,
               const glm::mat4 &projMat, const glm::mat4 &viewMat) override;
  void cleanup() override;
  void setupQuad();
  void resize(int width, int height) override;

private:
  std::unique_ptr<VertexArray> mQuadVAO;
  std::unique_ptr<VertexBuffer> mQuadVBO;
  int mTexColorBufferID;
};
