#include "Application.h"

#include "../core/Config.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "../gl/Debug.h"
#include "../rendering/LightManager.h"
#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "../resource/ResourceManager.h"
#include "../scene/Components.h"

#include <glad/glad.h>
#include <stb_image_write.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace {
/// Radians per second when rotating the sun with the arrow keys.
constexpr float kSunRotationSpeed = 0.6f;

/// Frames rendered before an automatic screenshot, so that everything
/// (window size, first shadow maps) has settled.
constexpr int kScreenshotWarmupFrames = 5;

std::string timestampedScreenshotName() {
  const std::time_t now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::ostringstream name;
  name << "screenshot_" << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S")
       << ".png";
  return name.str();
}
} // namespace

Application::Application(std::string title, ApplicationOptions options)
    : m_Title(std::move(title)), m_Options(std::move(options)) {
  Logger::init();
  Config::load(Config::getRootPath() + "config.txt");

  m_Width = Config::getWindowWidth();
  m_Height = Config::getWindowHeight();

  Initialize();
}

Application::~Application() { Cleanup(); }

void Application::Initialize() {
  if (!glfwInit())
    throw RendererException("Failed to initialize GLFW");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    throw RendererException("Failed to create the window, OpenGL 3.3 core is "
                            "required");
  }

  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(Config::getVSyncEnabled() ? 1 : 0);

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, FrameBufferSizeCallback);
  glfwSetCursorPosCallback(m_Window, MouseCallback);
  glfwSetScrollCallback(m_Window, ScrollCallback);
  glfwSetKeyCallback(m_Window, KeyCallback);
  glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    throw RendererException("Failed to load OpenGL functions");

  Logger::get()->info("OpenGL {} on {}",
                      reinterpret_cast<const char *>(glGetString(GL_VERSION)),
                      reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  enableGLDebugging();

  // The framebuffer can differ from the window size on high DPI screens
  glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
  m_Renderer = std::make_unique<ForwardRenderer>(m_Width, m_Height);
  if (m_Options.hasSunOverride)
    LightManager::getInstance().setSun(m_Options.sunAzimuth,
                                       m_Options.sunElevation);
  LightManager::getInstance().setLocalLightsEnabled(!m_Options.sunOnly);
  m_ShowCascades = m_Options.showCascades;
  m_Renderer->setShowCascades(m_ShowCascades);

  if (m_Options.modelPath.empty())
    SetupShadowTestScene();
  else
    SetupModelScene(m_Options.modelPath, m_Options.modelScale,
                    m_Options.modelImport);

  if (m_Options.hasCameraOverride)
    m_Camera = Camera(m_Options.cameraPosition, glm::vec3(0.0f, 1.0f, 0.0f),
                      m_Options.cameraYaw, m_Options.cameraPitch);
  m_Camera.mMovementSpeed = Config::getCameraSpeed();
  m_Camera.mMouseSensitivity = Config::getCameraSensitivity();
  m_Camera.mZoom = Config::getCameraFOV();
}

void Application::Run() {
  int frame = 0;
  while (!glfwWindowShouldClose(m_Window)) {
    const auto currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    ProcessInput();
    UpdateWindowTitle();
    m_Scene.onUpdate(m_DeltaTime);

    // Nothing to draw into while the window is minimized
    if (m_Width > 0 && m_Height > 0) {
      Render();

      if (m_ScreenshotRequested) {
        SaveScreenshot(Config::getScreenshotPath() +
                       timestampedScreenshotName());
        m_ScreenshotRequested = false;
      }
      if (!m_Options.screenshotPath.empty() &&
          ++frame == kScreenshotWarmupFrames) {
        SaveScreenshot(m_Options.screenshotPath);
        glfwSetWindowShouldClose(m_Window, true);
      }
    }

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
  }
}

void Application::ProcessInput() {
  m_InputHandler.processInput(m_Window, m_Camera, m_DeltaTime);

  auto &lights = LightManager::getInstance();
  if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS)
    lights.rotateSun(kSunRotationSpeed * m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    lights.rotateSun(-kSunRotationSpeed * m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS)
    lights.tiltSun(kSunRotationSpeed * m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS)
    lights.tiltSun(-kSunRotationSpeed * m_DeltaTime);
}

void Application::UpdateWindowTitle() {
  // Refreshed a few times per second, setting the title every frame is slow
  // on some window managers
  m_TitleTimer += m_DeltaTime;
  ++m_TitleFrames;
  if (m_TitleTimer < 0.25f)
    return;

  const auto &lights = LightManager::getInstance();
  char title[256];
  std::snprintf(title, sizeof(title),
                "%s | %.0f fps | sun azimuth %.0f, elevation %.0f",
                m_Title.c_str(), m_TitleFrames / m_TitleTimer,
                lights.getSunAzimuth(), lights.getSunElevation());
  glfwSetWindowTitle(m_Window, title);
  m_TitleTimer = 0.0f;
  m_TitleFrames = 0;
}

FrameContext Application::BuildFrameContext() const {
  FrameContext frame;
  frame.fovY = glm::radians(m_Camera.mZoom);
  frame.aspectRatio =
      static_cast<float>(m_Width) / static_cast<float>(m_Height);
  frame.nearPlane = Config::getCameraNearPlane();
  frame.farPlane = Config::getCameraFarPlane();
  frame.view = m_Camera.getViewMatrix();
  frame.projection = glm::perspective(frame.fovY, frame.aspectRatio,
                                      frame.nearPlane, frame.farPlane);
  frame.cameraPosition = m_Camera.mPosition;
  return frame;
}

void Application::Render() {
  m_Renderer->submit(m_Scene);
  m_Renderer->render(BuildFrameContext());
}

void Application::SaveScreenshot(const std::string &path) const {
  std::vector<unsigned char> pixels(static_cast<size_t>(m_Width) * m_Height *
                                    3);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE,
               pixels.data());

  const std::filesystem::path file(path);
  if (file.has_parent_path())
    std::filesystem::create_directories(file.parent_path());

  // OpenGL rows start at the bottom of the image
  stbi_flip_vertically_on_write(1);
  if (stbi_write_png(path.c_str(), m_Width, m_Height, 3, pixels.data(),
                     m_Width * 3))
    Logger::get()->info("Screenshot saved to {}", path);
  else
    Logger::get()->error("Could not write screenshot {}", path);
}

void Application::FrameBufferSizeCallback(GLFWwindow *window, const int width,
                                          const int height) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  app->m_Width = width;
  app->m_Height = height;
  app->m_Renderer->resize(width, height);
}

void Application::MouseCallback(GLFWwindow *window, const double xPos,
                                const double yPos) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  app->m_InputHandler.handleMouseMovement(xPos, yPos, app->m_Camera);
}

void Application::ScrollCallback(GLFWwindow *window, double /*xOffset*/,
                                 const double yOffset) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  app->m_InputHandler.handleMouseScroll(yOffset, app->m_Camera);
}

void Application::KeyCallback(GLFWwindow *window, const int key,
                              int /*scancode*/, const int action,
                              int /*mods*/) {
  if (action != GLFW_PRESS)
    return;

  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  switch (key) {
  case GLFW_KEY_C:
    app->m_ShowCascades = !app->m_ShowCascades;
    app->m_Renderer->setShowCascades(app->m_ShowCascades);
    break;
  case GLFW_KEY_L: {
    auto &lights = LightManager::getInstance();
    lights.setLocalLightsEnabled(!lights.areLocalLightsEnabled());
    break;
  }
  case GLFW_KEY_F12:
    app->m_ScreenshotRequested = true;
    break;
  default:
    break;
  }
}

void Application::SetupShadowTestScene() {
  // Backpacks spread along -Z so that every cascade has something to show
  const std::string backpack = Config::getModelPath() + "backpack/backpack.obj";
  ModelImportOptions backpackImport;
  backpackImport.flipUVs = true; // authored with a top-left UV origin
  constexpr int kRows = 6;
  for (int row = 0; row < kRows; ++row) {
    for (int column = -1; column <= 1; ++column) {
      Transform transform;
      const auto distance = static_cast<float>(row * row) * 2.5f;
      transform.position = {static_cast<float>(column) * 3.0f + 0.2f * row,
                            0.0f, -distance};
      transform.rotation.y = glm::radians(35.0f * (row + column));
      transform.scale = glm::vec3(0.5f);
      ResourceManager::instantiateModel(m_Scene, backpack, transform,
                                        backpackImport);
    }
  }

  // One raised and tilted to check shadows cast onto other objects
  Transform floating;
  floating.position = {0.0f, 1.2f, 1.0f};
  floating.rotation.x = glm::radians(15.0f);
  floating.scale = glm::vec3(0.4f);
  ResourceManager::instantiateModel(m_Scene, backpack, floating,
                                    backpackImport);

  CreateGroundPlane(100.0f);

  m_Camera = Camera({0.0f, 2.0f, 7.0f}, {0.0f, 1.0f, 0.0f}, -90.0f, -12.0f);
}

void Application::SetupModelScene(const std::string &path, const float scale,
                                  const ModelImportOptions &import) {
  Transform transform;
  transform.scale = glm::vec3(scale);
  ResourceManager::instantiateModel(m_Scene, path, transform, import);

  // Look at the model from its front (+Z side), slightly from above
  const auto model = ResourceManager::loadModel(path, import);
  const glm::vec3 center = (model->boundsMin + model->boundsMax) * 0.5f * scale;
  const float radius =
      glm::length(model->boundsMax - model->boundsMin) * 0.5f * scale;
  const glm::vec3 position =
      center + glm::vec3(0.0f, 0.4f, 1.0f) * radius * 1.6f;
  const glm::vec3 toCenter = glm::normalize(center - position);
  m_Camera = Camera(position, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f,
                    glm::degrees(std::asin(toCenter.y)));
}

void Application::CreateGroundPlane(const float halfSize) {
  constexpr float y = -1.0f;
  const float uvScale = halfSize / 2.0f;
  const glm::vec3 up(0.0f, 1.0f, 0.0f);
  const glm::vec3 tangent(1.0f, 0.0f, 0.0f);
  const glm::vec3 bitangent(0.0f, 0.0f, -1.0f);

  const std::vector<Vertex> vertices = {
      {{-halfSize, y, halfSize}, up, tangent, bitangent, {0.0f, 0.0f}},
      {{halfSize, y, halfSize}, up, tangent, bitangent, {uvScale, 0.0f}},
      {{halfSize, y, -halfSize}, up, tangent, bitangent, {uvScale, uvScale}},
      {{-halfSize, y, -halfSize}, up, tangent, bitangent, {0.0f, uvScale}}};
  const std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

  auto material = std::make_shared<Material>();
  material->setAlbedo(glm::vec3(0.25f));
  material->setSpecular(glm::vec3(0.05f));

  Entity ground = m_Scene.createEntity("Ground");
  ground.addComponent<Transform>();
  ground.addComponent<MeshRenderer>(std::make_shared<Mesh>(vertices, indices),
                                    material);
}

void Application::Cleanup() {
  // GPU objects must be released while the context is still alive: the
  // renderer first, then everything the scene and the caches keep alive.
  m_Renderer.reset();
  m_Scene.clear();
  ResourceManager::clearCache();
  Material::releaseDefaultTextures();

  if (m_Window) {
    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
  }
  glfwTerminate();
}
