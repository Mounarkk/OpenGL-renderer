#include "Application.h"

#include "../gl/Debug.h"
#include "../gl/GLObjectDestroyer.h"
#include "../resource/ModelLoader.h"

#include <glad/glad.h>

#include <utility>

Application::Application(std::string title) : m_Title(std::move(title)) {
  // Initialize logger first
  Logger::init();
  
  // Load configuration first
  Config::load("../config.txt");  // The path needs to be set relative to the executable
  
  // Use configuration values
  m_Width = Config::getWindowWidth();
  m_Height = Config::getWindowHeight();
  
  // Initialize camera with configured settings
  m_Camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
  m_Camera.mMovementSpeed = Config::getCameraSpeed();
  m_Camera.mMouseSensitivity = Config::getCameraSensitivity();
  m_Camera.mZoom = Config::getCameraFOV();
  
  // Initialize input handler with window dimensions
  m_InputHandler.initialize(m_Width, m_Height);
  
  Initialize();
}

Application::Application(const int width, const int height, std::string title)
    : m_Width(width), m_Height(height), m_Title(std::move(title)),
      m_Camera(glm::vec3(0.0f, 0.0f, 3.0f)) {
  // Initialize logger first
  Logger::init();
  
  // Load configuration for paths and other settings
  Config::load("config.txt");  // The path needs to be set relative to the executable
  
  // Initialize input handler with window dimensions
  m_InputHandler.initialize(width, height);
  
  Initialize();
}

Application::~Application() {
  GLObjectDestroyer::getInstance().cleanupAll();
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void Application::Initialize() {
  Logger::get()->info("Starting application...");

  // Initialize GLFW library for window and input management
  if (!glfwInit()) {
    Logger::get()->error("Failed to initialize GLFW");
    return;
  }

  // Configure OpenGL context before window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);              // Request OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use core profile (no deprecated features)

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);         // Required for macOS compatibility
#endif

  // Create a window
  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    Logger::get()->error("Failed to create GLFW window");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(m_Window);                                    // Make this window's context current for OpenGL calls
  glfwSetWindowUserPointer(m_Window, this);                           // Store 'this' pointer for callback access
  glfwSetFramebufferSizeCallback(m_Window, FrameBufferSizeCallback);   // Register window resize callback
  glfwSetCursorPosCallback(m_Window, MouseCallback);                   // Register mouse movement callback
  glfwSetScrollCallback(m_Window, ScrollCallback);                     // Register mouse scroll callback

  // Capture mouse cursor for FPS-style camera controls
  glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Load OpenGL function pointers using GLAD
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    Logger::get()->error("Failed to initialize GLAD");
    return;
  }

  m_Renderer = std::make_unique<Renderer>();

  // Enable depth testing for proper 3D rendering (closer objects hide farther ones)
  glEnable(GL_DEPTH_TEST);

  // Enable debug out put
  enableGLDebugging();

  // Initialize scene
  // TODO: Default scene here
  std::string backpackPath = Config::getModelPath() + "backpack/backpack.obj";
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
  // Delegate all input processing to the InputHandler
  m_InputHandler.processInput(m_Window, m_Camera, m_DeltaTime);
}

void Application::Update(const float deltaTime) {
  // Update scene (e.g., animations, physics)
  m_Scene.onUpdate(deltaTime);
}

void Application::Render() {
  // Clear screen
  m_Renderer->clear();

  // Set camera matrices
  const glm::mat4 view = m_Camera.getViewMatrix();
  const glm::mat4 projection = glm::perspective(
      glm::radians(m_Camera.mZoom),
      static_cast<float>(m_Width) / static_cast<float>(m_Height), 
      Config::getCameraNearPlane(), Config::getCameraFarPlane());

  // Submit render commands
  RenderingSystem::onUpdate(m_Scene, *m_Renderer);

  // Flush render commands
  m_Renderer->flush(projection * view);
}

// Static callbacks
void Application::FrameBufferSizeCallback(GLFWwindow *window, const int width,
                                          const int height) {
  glViewport(0, 0, width, height);
  // Update renderer with new dimensions
  Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (app && app->m_Renderer) {
    app->m_Renderer->resize(width, height);
  }
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
  // Delegate mouse movement handling to InputHandler
  m_InputHandler.handleMouseMovement(xPos, yPos, m_Camera);
}

void Application::HandleScrollInput(double xOffset, const double yOffset) {
  // Delegate scroll handling to InputHandler
  m_InputHandler.handleMouseScroll(xOffset, yOffset, m_Camera);
}