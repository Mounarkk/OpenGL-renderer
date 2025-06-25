#pragma once
#include "../core/Logger.h"
#include "../rendering/Renderer.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"
#include "../systems/RenderingSystem.h"
#include <GLFW/glfw3.h>

class Application {
public:
  Application(int width, int height, std::string title);
  ~Application();

  void Run();

private:
  void Initialize();
  void ProcessInput();
  void Update(float deltaTime);
  void Render();

  // Callbacks
  static void FrameBufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void MouseCallback(GLFWwindow *window, double xPos, double yPos);
  static void ScrollCallback(GLFWwindow *window, double xOffset,
                             double yOffset);
  void HandleMouseInput(double xPos, double yPos);
  void HandleScrollInput(double xOffset, double yOffset);

  // Window and input
  GLFWwindow *m_Window;
  int m_Width, m_Height;
  std::string m_Title;

  // Camera
  Camera m_Camera;
  float m_LastX, m_LastY;
  bool m_FirstMouse = true;

  // Timing
  float m_DeltaTime = 0.0f;
  float m_LastFrame = 0.0f;

  // Scene and systems
  Scene m_Scene;
  std::unique_ptr<ForwardRenderer> m_Renderer;
};