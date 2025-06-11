#include "Application.h"

#include "../gl/Debug.h"
#include "../gl/GLObjectDestroyer.h"
#include "../resource/ModelLoader.h"

#include <glad/glad.h>

#include <utility>

Application::Application(const int width, const int height, std::string title)
    : m_Width(width), m_Height(height), m_Title(std::move(title)),
      m_Camera(glm::vec3(0.0f, 0.0f, 3.0f)),
      m_LastX((static_cast<float>(width) / 2.0f)),
      m_LastY(static_cast<float>(height) / 2.0f) {
  Initialize();
}

Application::~Application() {
  GLObjectDestroyer::getInstance().cleanupAll();
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void Application::Initialize() {
  Logger::init();
  Logger::get()->info("Starting application...");

  // GLFW initialization
  if (!glfwInit()) {
    Logger::get()->error("Failed to initialize GLFW");
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Create a window
  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    Logger::get()->error("Failed to create GLFW window");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(m_Window);
  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, FrameBufferSizeCallback);
  glfwSetCursorPosCallback(m_Window, MouseCallback);
  glfwSetScrollCallback(m_Window, ScrollCallback);

  // Capture mouse
  glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Load OpenGL functions
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    Logger::get()->error("Failed to initialize GLAD");
    return;
  }

  m_Renderer = std::make_unique<Renderer>();

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Enable debug out put
  enableGLDebugging();

  // Initialize scene
  // TODO: Default scene here
  std::string backpackPath = "../res/models/backpack/backpack.obj";
  ModelLoader::load(m_Scene, backpackPath);
}

void Application::Run() {
  while (!glfwWindowShouldClose(m_Window)) {
    // Update time
    const auto currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    // Input
    ProcessInput();

    // Update
    Update(m_DeltaTime);

    // Render
    Render();

    // Swap buffers and poll events
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
  }
}

void Application::ProcessInput() {
  if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_Window, true);

  if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    m_Camera.processKeyboard(FORWARD, m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    m_Camera.processKeyboard(BACKWARD, m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
    m_Camera.processKeyboard(LEFT, m_DeltaTime);
  if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
    m_Camera.processKeyboard(RIGHT, m_DeltaTime);
}

void Application::Update(const float deltaTime) {
  // Update scene (e.g., animations, physics)
  m_Scene.onUpdate(deltaTime);
}

void Application::Render() {
  // Clear screen
  Renderer::clear();

  // Set camera matrices
  const glm::mat4 view = m_Camera.getViewMatrix();
  const glm::mat4 projection = glm::perspective(
      glm::radians(m_Camera.mZoom),
      static_cast<float>(m_Width) / static_cast<float>(m_Height), 0.1f, 100.0f);

  // Submit render commands
  RenderingSystem::onUpdate(m_Scene);

  // Flush render commands
  m_Renderer->flush(projection * view);
}

// Static callbacks
void Application::FrameBufferSizeCallback(GLFWwindow *window, const int width,
                                          const int height) {
  glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow *window, const double xPos,
                                const double yPos) {
  static_cast<Application *>(glfwGetWindowUserPointer(window))
      ->HandleMouseInput(xPos, yPos);
}

void Application::ScrollCallback(GLFWwindow *window, const double xOffset,
                                 const double yOffset) {
  static_cast<Application *>(glfwGetWindowUserPointer(window))
      ->HandleScrollInput(xOffset, yOffset);
}

void Application::HandleMouseInput(const double xPos, const double yPos) {
  if (m_FirstMouse) {
    m_LastX = static_cast<float>(xPos);
    m_LastY = static_cast<float>(yPos);
    m_FirstMouse = false;
  }

  const float xOffset = static_cast<float>(xPos) - m_LastX;
  const float yOffset =
      m_LastY - static_cast<float>(
                    yPos); // Reversed since y-coordinates go from bottom to top
  m_LastX = static_cast<float>(xPos);
  m_LastY = static_cast<float>(yPos);

  m_Camera.processMouseMovement(xOffset, yOffset);
}

void Application::HandleScrollInput(double xOffset, const double yOffset) {
  m_Camera.processMouseScroll(static_cast<float>(yOffset));
}