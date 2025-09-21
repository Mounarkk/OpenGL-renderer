#pragma once
#include "../core/Logger.h"
#include "../core/Config.h"
#include "../rendering/Renderer.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"
#include "../systems/RenderingSystem.h"
#include "../input/InputHandler.h"
#include <GLFW/glfw3.h>

/**
 * Main application class that manages the window, input, and main loop.
 * 
 * The Application class serves as the central coordinator for the entire renderer.
 * It manages the GLFW window, processes user input (keyboard and mouse), 
 * coordinates the scene and renderer, and runs the main game loop.
 * 
 * Key responsibilities:
 * - Window creation and management
 * - Input processing and camera controls
 * - Main loop timing and coordination
 * - Scene and renderer lifecycle management
 */
class Application {
public:
  /**
   * Constructs the application with default settings from configuration.
   * Window dimensions and other settings are loaded from the config file.
   * @param title Window title displayed in the title bar
   */
  explicit Application(std::string title);
  
  /**
   * Constructs the application with specified window dimensions and title.
   * @param width Initial window width in pixels
   * @param height Initial window height in pixels  
   * @param title Window title displayed in the title bar
   */
  Application(int width, int height, std::string title);
  
  /**
   * Destructor that cleans up GLFW resources and terminates the application.
   */
  ~Application();

  /**
   * Starts the main application loop.
   * This method initializes all systems, then runs the main loop until
   * the user closes the window. Handles timing, input, updates, and rendering.
   */
  void Run();

private:
  /**
   * Initializes GLFW, creates the window, and sets up OpenGL context.
   * Also initializes GLAD and sets up initial OpenGL state.
   */
  void Initialize();
  
  /**
   * Processes all input using the InputHandler.
   * Delegates input processing to the InputHandler instance.
   */
  void ProcessInput();
  
  /**
   * Updates the scene and all game systems.
   * @param deltaTime Time elapsed since the last frame in seconds
   */
  void Update(float deltaTime);
  
  /**
   * Renders the current frame.
   * Coordinates between the scene and renderer to draw all visible objects.
   */
  void Render();

  // GLFW Callbacks - these must be static for GLFW compatibility
  
  /**
   * GLFW callback for window resize events.
   * Updates the OpenGL viewport when the window is resized.
   * @param window The GLFW window that was resized
   * @param width New window width in pixels
   * @param height New window height in pixels
   */
  static void FrameBufferSizeCallback(GLFWwindow *window, int width, int height);
  
  /**
   * GLFW callback for mouse movement events.
   * @param window The GLFW window receiving the event
   * @param xPos Mouse X position in screen coordinates
   * @param yPos Mouse Y position in screen coordinates
   */
  static void MouseCallback(GLFWwindow *window, double xPos, double yPos);
  
  /**
   * GLFW callback for mouse scroll events.
   * @param window The GLFW window receiving the event
   * @param xOffset Horizontal scroll offset (usually 0)
   * @param yOffset Vertical scroll offset (positive = scroll up)
   */
  static void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
  
  /**
   * Handles mouse movement by delegating to InputHandler.
   * @param xPos Current mouse X position
   * @param yPos Current mouse Y position
   */
  void HandleMouseInput(double xPos, double yPos);
  
  /**
   * Handles mouse scroll by delegating to InputHandler.
   * @param xOffset Horizontal scroll offset (unused)
   * @param yOffset Vertical scroll offset for zoom
   */
  void HandleScrollInput(double xOffset, double yOffset);

  // Window and input
  GLFWwindow *m_Window;
  int m_Width, m_Height;
  std::string m_Title;

  // Camera and input
  Camera m_Camera;
  InputHandler m_InputHandler;

  // Timing
  float m_DeltaTime = 0.0f;
  float m_LastFrame = 0.0f;

  // Scene and systems
  Scene m_Scene;
  std::unique_ptr<Renderer> m_Renderer;
};