#include "Application.h"

#include "../core/Config.h"
#include "../core/Logger.h"
#include "../core/RendererException.h"
#include "../gl/Debug.h"
#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "../rendering/Primitives.h"
#include "../resource/ResourceManager.h"
#include "../scene/Components.h"

#include <glad/glad.h>
#include <stb_image_write.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
/// Degrees per second when moving the sun with the arrow keys.
constexpr float kSunRotationSpeed = 35.0f;

/// Keeps the sun between the horizon and the zenith. Below a couple of
/// degrees shadows become extremely long, at the zenith the light view basis
/// of the cascades is degenerate.
constexpr float kMinSunElevation = 2.0f;
constexpr float kMaxSunElevation = 89.0f;

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

  // 4.5 for direct state access, 4.3 would be enough for the rest
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    throw RendererException("Failed to create the window, OpenGL 4.5 core is "
                            "required");
  }

  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(Config::getVSyncEnabled() ? 1 : 0);

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, FrameBufferSizeCallback);
  glfwSetCursorPosCallback(m_Window, MouseCallback);
  glfwSetScrollCallback(m_Window, ScrollCallback);
  glfwSetKeyCallback(m_Window, KeyCallback);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    throw RendererException("Failed to load OpenGL functions");

  Logger::get()->info("OpenGL {} on {}",
                      reinterpret_cast<const char *>(glGetString(GL_VERSION)),
                      reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  enableGLDebugging();

  // The framebuffer can differ from the window size on high DPI screens
  glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
  m_Renderer = std::make_unique<ForwardRenderer>(m_Width, m_Height);
  m_Renderer->getSettings().localLightsEnabled = !m_Options.sunOnly;
  m_Renderer->getSettings().showCascades = m_Options.showCascades;

  // After our callbacks are installed: the ImGui backend chains to them
  m_DebugUI = std::make_unique<DebugUI>(m_Window);
  SetCameraMode(true);

  if (m_Options.modelPath.empty())
    SetupShadowTestScene();
  else
    SetupModelScene(m_Options.modelPath, m_Options.modelScale,
                    m_Options.modelImport);

  if (m_Options.hasSunOverride) {
    auto *sun = FindSun();
    sun->azimuth = m_Options.sunAzimuth;
    sun->elevation =
        glm::clamp(m_Options.sunElevation, kMinSunElevation, kMaxSunElevation);
  }

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

    m_DebugUI->build({m_Renderer->getSettings(), m_Renderer->getStats(),
                      m_Scene, m_Camera, m_CameraMode, m_ScreenshotRequested});

    // Nothing to draw into while the window is minimized
    if (m_Width > 0 && m_Height > 0) {
      Render();

      // Screenshots are taken before the panel is drawn over the frame

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
    m_DebugUI->render();

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
  }
}

void Application::ProcessInput() {
  // Keys typed into a text field of the panel must not move the camera
  if (m_DebugUI->wantsKeyboard())
    return;

  m_InputHandler.processInput(m_Window, m_Camera, m_DeltaTime);

  DirectionalLight *sun = FindSun();
  if (!sun)
    return;

  const float step = kSunRotationSpeed * m_DeltaTime;
  if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS)
    sun->azimuth = std::fmod(sun->azimuth + step, 360.0f);
  if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    sun->azimuth = std::fmod(sun->azimuth - step + 360.0f, 360.0f);
  if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS)
    sun->elevation = std::min(sun->elevation + step, kMaxSunElevation);
  if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS)
    sun->elevation = std::max(sun->elevation - step, kMinSunElevation);
}

DirectionalLight *Application::FindSun() {
  for (const auto entity : m_Scene.getAll<DirectionalLight>())
    return &m_Scene.getRegistry().get<DirectionalLight>(entity);
  return nullptr;
}

void Application::UpdateWindowTitle() {
  // Refreshed a few times per second, setting the title every frame is slow
  // on some window managers
  m_TitleTimer += m_DeltaTime;
  ++m_TitleFrames;
  if (m_TitleTimer < 0.25f)
    return;

  char title[256];
  const DirectionalLight *sun = FindSun();
  if (sun)
    std::snprintf(title, sizeof(title),
                  "%s | %.0f fps | sun azimuth %.0f, elevation %.0f",
                  m_Title.c_str(), m_TitleFrames / m_TitleTimer, sun->azimuth,
                  sun->elevation);
  else
    std::snprintf(title, sizeof(title), "%s | %.0f fps", m_Title.c_str(),
                  m_TitleFrames / m_TitleTimer);
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
  if (app->m_CameraMode)
    app->m_InputHandler.handleMouseMovement(xPos, yPos, app->m_Camera);
}

void Application::ScrollCallback(GLFWwindow *window, double /*xOffset*/,
                                 const double yOffset) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app->m_CameraMode)
    app->m_InputHandler.handleMouseScroll(yOffset, app->m_Camera);
}

void Application::KeyCallback(GLFWwindow *window, const int key,
                              int /*scancode*/, const int action,
                              int /*mods*/) {
  if (action != GLFW_PRESS)
    return;

  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app->m_DebugUI && app->m_DebugUI->wantsKeyboard())
    return;

  auto &settings = app->m_Renderer->getSettings();
  switch (key) {
  case GLFW_KEY_TAB:
    app->SetCameraMode(!app->m_CameraMode);
    break;
  case GLFW_KEY_F1:
    app->m_DebugUI->toggleVisible();
    break;
  case GLFW_KEY_C:
    settings.showCascades = !settings.showCascades;
    break;
  case GLFW_KEY_L:
    settings.localLightsEnabled = !settings.localLightsEnabled;
    break;
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
  CreateSun();
  CreateTestLights();

  m_Camera = Camera({0.0f, 2.0f, 7.0f}, {0.0f, 1.0f, 0.0f}, -90.0f, -12.0f);
}

void Application::SetupModelScene(const std::string &path, const float scale,
                                  const ModelImportOptions &import) {
  Transform transform;
  transform.scale = glm::vec3(scale);
  ResourceManager::instantiateModel(m_Scene, path, transform, import);
  CreateSun();

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
  auto material = std::make_shared<Material>();
  material->setAlbedo(glm::vec3(0.25f));
  material->setSpecular(glm::vec3(0.05f));

  Transform transform;
  transform.position.y = -1.0f;

  Entity ground = m_Scene.createEntity("Ground");
  ground.addComponent<Transform>(transform);
  ground.addComponent<MeshRenderer>(
      Primitives::createPlane(halfSize, halfSize / 2.0f), material);
}

void Application::CreateSun() {
  Entity sun = m_Scene.createEntity("Sun");
  sun.addComponent<DirectionalLight>();
}

void Application::CreateTestLights() {
  // Spread over the test scene, slightly tinted to tell them apart
  const glm::vec3 positions[] = {{1.5f, 0.3f, 1.5f},
                                 {-2.5f, 0.5f, -3.0f},
                                 {2.5f, 1.0f, -9.0f},
                                 {-1.5f, 1.5f, -20.0f}};
  const glm::vec3 colors[] = {{1.0f, 0.75f, 0.5f},
                              {0.5f, 0.7f, 1.0f},
                              {0.6f, 1.0f, 0.6f},
                              {1.0f, 1.0f, 1.0f}};
  for (size_t i = 0; i < std::size(positions); ++i) {
    Entity light = m_Scene.createEntity("Point light " + std::to_string(i));
    Transform transform;
    transform.position = positions[i];
    light.addComponent<Transform>(transform);
    PointLight point;
    point.color = colors[i];
    light.addComponent<PointLight>(point);
  }

  // Above and behind the camera, aimed down at the front row
  Entity spot = m_Scene.createEntity("Spot light");
  Transform transform;
  transform.position = {0.0f, 3.0f, 5.0f};
  transform.rotation.x = glm::radians(-31.0f);
  spot.addComponent<Transform>(transform);
  spot.addComponent<SpotLight>();
}

void Application::SetCameraMode(const bool enabled) {
  m_CameraMode = enabled;
  glfwSetInputMode(m_Window, GLFW_CURSOR,
                   enabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  m_DebugUI->setMouseEnabled(!enabled);
  if (enabled)
    m_InputHandler.resetMouse();
}

void Application::Cleanup() {
  // GPU objects must be released while the context is still alive: the
  // renderer first, then everything the scene and the caches keep alive.
  m_DebugUI.reset();
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
