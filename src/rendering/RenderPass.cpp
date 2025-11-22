#include "RenderPass.h"
#include "../resource/ResourceManager.h"
#include "LightManager.h"

ShadowMappingPass::ShadowMappingPass(const std::string &vertexPath,
                                         const std::string &fragmentPath,
                                         int depthMapWidth, int depthMapHeight) {
  mShader = ResourceManager::loadShader(vertexPath, fragmentPath);
  auto type = FrameBufferType::ShadowMap;
  mFBO = std::make_unique<FrameBuffer>(depthMapWidth, depthMapHeight, type);
}

void ShadowMappingPass::execute(std::vector<RenderCommand> &s_CommandQueue,
                                  const glm::mat4 &projMat,
                                  const glm::mat4 &viewMat) {

  // Configure directional light matrices FIRST
  constexpr float nearPlane = 1.0f;
  constexpr float farPlane = 7.5f;
  const glm::mat4 dirLightProjection = glm::ortho(-10.f, 10.f,
                                          -10.f, 10.f,
                                          nearPlane, farPlane);
  const glm::mat4 dirLightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), // position
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));

  mDirLightProjView = dirLightProjection * dirLightView;

  // Set up the pass
  const unsigned int width = mFBO->getWidth();
  const unsigned int height = mFBO->getHeight();
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);

  mShader->use();
  mShader->setMat4("uDirLightViewProj", mDirLightProjView);

  // Batch draw calls
  for (const auto &[transform, mesh, material] : s_CommandQueue) {
    mShader->setMat4("uModel", transform.getWorldMatrix());
    mesh->draw();
  }

  mFBO->unbind();
}

void ShadowMappingPass::cleanup() {
  mFBO->clean();
}

void ShadowMappingPass::resize(int width, int height) {
  mFBO->clean();
  auto type = FrameBufferType::ShadowMap;
  mFBO = std::make_unique<FrameBuffer>(width, height, type);
}

ShadowMappingPass::~ShadowMappingPass() { cleanup(); }

int ShadowMappingPass::getTextDepthBufferID() const {
  return mFBO->getDepthTexture();
}

glm::mat4 ShadowMappingPass::getDirLightProjView() const {
  return mDirLightProjView;
}

ForwardLightingPass::ForwardLightingPass(const std::string &vertexPath,
                                         const std::string &fragmentPath,
                                         const int width, const int height,
                                         const int dirLightShadowMapBufferID,
                                         const glm::mat4 &dirLightProjView) {
  mDirLightProjView = dirLightProjView;
  mDirLightShadowMapBufferID = dirLightShadowMapBufferID;
  mShader = ResourceManager::loadShader(vertexPath, fragmentPath);
  mFBO = std::make_unique<FrameBuffer>(width, height);
  mSkybox = std::make_unique<Skybox>();
}

void ForwardLightingPass::execute(std::vector<RenderCommand> &s_CommandQueue,
                                  const glm::mat4 &projMat,
                                  const glm::mat4 &viewMat) {

  // Set up the pass
  const unsigned int width = mFBO->getWidth();
  const unsigned int height = mFBO->getHeight();
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  mFBO->bind();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mShader->use();
  mShader->setMat4("uViewProj", projMat * viewMat);
  mShader->setMat4("uDirLightViewProj", mDirLightProjView);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mDirLightShadowMapBufferID);
  mShader->setInt("uDirLightShadowMap", 0); // texture unit number, not texture ID

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

void ForwardLightingPass::cleanup() {
  mFBO->clean();
  // mShader is now managed by ResourceManager via shared_ptr, no manual cleanup
  // needed
}

void ForwardLightingPass::resize(int width, int height) {
  mFBO->clean();
  mFBO = std::make_unique<FrameBuffer>(width, height);
}

int ForwardLightingPass::getTextColorBufferID() const {
  return mFBO->mTextureID;
}

ForwardLightingPass::~ForwardLightingPass() { cleanup(); }

PostProcessingPass::PostProcessingPass(const std::string &vertexPath,
                                       const std::string &fragmentPath,
                                       const int texColorBufferID) {
  mTexColorBufferID = texColorBufferID;
  mShader = ResourceManager::loadShader(vertexPath, fragmentPath);
  setupQuad();
}

void PostProcessingPass::execute(std::vector<RenderCommand> &,
                                 const glm::mat4 &, const glm::mat4 &) {

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

void PostProcessingPass::cleanup() {
  mQuadVAO->clean();
  mQuadVBO->clean();
  // mShader is now managed by ResourceManager via shared_ptr, no manual cleanup
  // needed
}

void PostProcessingPass::setupQuad() {
  float quadVertices[] = {// positions   // texCoords
                          -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                          0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                          1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  mQuadVBO = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));

  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 2);
  layout.Push(GL_FLOAT, 2);

  mQuadVAO = std::make_unique<VertexArray>();
  mQuadVAO->addBuffer(*mQuadVBO, layout);
}

void PostProcessingPass::resize(const int, const int) {
  // Post-processing pass doesn't need to resize
}

PostProcessingPass::~PostProcessingPass() { cleanup(); }