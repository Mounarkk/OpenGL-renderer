#include "RenderPass.h"
#include "LightManager.h"


ForwardRenderPass::ForwardRenderPass(const std::string &vertexPath,
                                     const std::string &fragmentPath,
                                     const int width, const int height) {
  mShader = std::make_unique<Shader>(vertexPath, fragmentPath);
  mFBO = std::make_unique<FrameBuffer>(width, height);
  mSkybox = std::make_unique<Skybox>();
}

void ForwardRenderPass::execute(std::vector<RenderCommand> &s_CommandQueue, const glm::mat4 &projMat, const glm::mat4 &viewMat) {

  // Set up the pass
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mShader->use();
  mShader->setMat4("uViewProj", projMat * viewMat);
  LightManager::getInstance().bindLights(mShader);

  // Batch draw calls
  for (const auto &[transform, mesh, material] : s_CommandQueue) {
    material->bind(mShader);
    mShader->setMat4("uModel", transform.getWorldMatrix());
    mesh->draw();
  }

  // Render the skybox
  mSkybox->render(projMat, viewMat);

  mFBO->unbind();
}

void ForwardRenderPass::cleanup() {
  mFBO->clean();
  mShader->clean();
}

void ForwardRenderPass::resize(int width, int height) {
  mFBO->clean();
  mFBO = std::make_unique<FrameBuffer>(width, height);
}

int ForwardRenderPass::getTextColorBufferID() const {
  return mFBO->mTextureID;
}

ForwardRenderPass::~ForwardRenderPass() {
  cleanup();
}


FinalRenderPass::FinalRenderPass(const std::string &vertexPath,
                                 const std::string &fragmentPath,
                                 const int texColorBufferID) {
  mTexColorBufferID = texColorBufferID;
  mShader = std::make_unique<Shader>(vertexPath, fragmentPath);
  setupQuad();
}

void FinalRenderPass::execute(std::vector<RenderCommand> &s_CommandQueue,
                              const glm::mat4 &projMat, const glm::mat4 &viewMat) {
  // Don't use the command queue/projection matrix
  (void)s_CommandQueue;
  (void)projMat;
  (void)viewMat;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  mShader->use();
  mQuadVBO->Bind();
  mQuadVAO->bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mTexColorBufferID);
  mShader->setInt("screenTexture", 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  VertexBuffer::Unbind();
  VertexArray::unbind();
}

void FinalRenderPass::cleanup() {
  mQuadVAO->clean();
  mQuadVBO->clean();
  mShader->clean();
}

void FinalRenderPass::setupQuad() {
  float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

  mQuadVBO = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));

  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 2);
  layout.Push(GL_FLOAT, 2);

  mQuadVAO = std::make_unique<VertexArray>();
  mQuadVAO->addBuffer(*mQuadVBO, layout);
}

void FinalRenderPass::resize(const int width, const int height) { return; }


FinalRenderPass::~FinalRenderPass() {
  cleanup();
}






