#pragma once
#include "../input/InputHandler.h"
#include "../rendering/Renderer.h"
#include "../resource/Model.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"

#include <GLFW/glfw3.h>

#include <memory>
#include <string>

/// Startup options, filled from the command line.
struct ApplicationOptions {
  /// When set, a few frames are rendered, saved to this PNG, and the
  /// application exits. Handy to produce screenshots from a script.
  std::string screenshotPath;

  /// Model to display instead of the default test scene.
  std::string modelPath;
  float modelScale = 1.0f;
  ModelImportOptions modelImport;

  /// Initial camera placement. When unset, the camera frames the model.
  bool hasCameraOverride = false;
  glm::vec3 cameraPosition{0.0f};
  float cameraYaw = -90.0f;
  float cameraPitch = 0.0f;

  /// Initial sun angles in degrees, the default light is kept when unset.
  bool hasSunOverride = false;
  float sunAzimuth = 0.0f;
  float sunElevation = 45.0f;

  /// Starts with the point and spot lights disabled.
  bool sunOnly = false;

  /// Starts with the cascade debug view enabled.
  bool showCascades = false;
};

/**
 * Owns the window, the scene and the renderer, and runs the main loop.
 *
 * Controls:
 *   WASD / Space / Ctrl   move, Shift to go faster
 *   Mouse / Scroll        look around / zoom
 *   Left / Right arrows   turn the sun around
 *   Up / Down arrows      raise / lower the sun
 *   L                     toggle the point and spot lights
 *   C                     toggle the shadow cascade debug view
 *   F12                   save a screenshot
 *   Escape                quit
 */
class Application {
public:
  explicit Application(std::string title, ApplicationOptions options = {});
  ~Application();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  void Run();

private:
  void Initialize();
  void ProcessInput();
  void UpdateWindowTitle();
  void Render();
  void Cleanup();

  [[nodiscard]] FrameContext BuildFrameContext() const;

  /// Reads the window back buffer and writes it as a PNG.
  void SaveScreenshot(const std::string &path) const;

  void SetupShadowTestScene();
  void SetupModelScene(const std::string &path, float scale,
                       const ModelImportOptions &import);
  void CreateGroundPlane(float halfSize);

  static void FrameBufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void MouseCallback(GLFWwindow *window, double xPos, double yPos);
  static void ScrollCallback(GLFWwindow *window, double xOffset,
                             double yOffset);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

  GLFWwindow *m_Window = nullptr;
  int m_Width;
  int m_Height;
  std::string m_Title;
  ApplicationOptions m_Options;

  Camera m_Camera;
  InputHandler m_InputHandler;
  Scene m_Scene;
  std::unique_ptr<Renderer> m_Renderer;

  float m_DeltaTime = 0.0f;
  float m_LastFrame = 0.0f;
  bool m_ShowCascades = false;
  bool m_ScreenshotRequested = false;

  float m_TitleTimer = 0.0f;
  int m_TitleFrames = 0;
};
