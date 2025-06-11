#pragma once
#include "../gl/Shader.h"
#include "../gl/FrameBuffer.h"
#include "Material.h"
#include "Mesh.h"
#include <memory>
#include "../scene/Components.h"

struct RenderCommand {
  Transform transform;
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
};


class RenderPass {
public:
  virtual ~RenderPass() = default;
  virtual void execute(std::vector<RenderCommand> &s_CommandQueue, const glm::mat4 &viewProj) = 0;
  virtual void cleanup() = 0;
  virtual void resize(const int width, const int height) = 0;
protected:
  std::shared_ptr<Shader> mShader;
};

class ForwardRenderPass final : protected RenderPass {
public:
  ForwardRenderPass(const std::string &vertexPath, const std::string &fragmentPath, int width, int height);
  ~ForwardRenderPass() override;

  void execute(std::vector<RenderCommand> &s_CommandQueue, const glm::mat4 &viewProj) override;
  void cleanup() override;
  void resize(const int width, const int height) override;

  [[nodiscard]] int getTextColorBufferID() const;
private:
  std::unique_ptr<FrameBuffer> mFBO;
};

class FinalRenderPass final : protected RenderPass {
public:
  FinalRenderPass(const std::string &vertexPath, const std::string &fragmentPath, const int texColorBufferID);
  ~FinalRenderPass() override;

  void execute(std::vector<RenderCommand> &s_CommandQueue, const glm::mat4 &viewProj) override;
  void cleanup() override;
  void setupQuad();
  void resize(const int width, const int height) override;
private:
  std::unique_ptr<VertexArray> mQuadVAO;
  std::unique_ptr<VertexBuffer> mQuadVBO;
  int mTexColorBufferID;
};

