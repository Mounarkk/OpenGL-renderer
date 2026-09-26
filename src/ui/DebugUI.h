#pragma once
#include "../rendering/RendererSettings.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"

struct GLFWwindow;

/// What the debug panel reads and edits during a frame.
struct DebugUIContext {
  RendererSettings &settings;
  const RenderStats &stats;
  Scene &scene;
  Camera &camera;
  bool cameraMode; ///< True while the mouse drives the camera
  bool &screenshotRequested;
};

/**
 * Dear ImGui panel to inspect and tweak the renderer live.
 *
 * Must be initialized after the application installed its own GLFW
 * callbacks: the ImGui backend chains to them.
 */
class DebugUI {
public:
  explicit DebugUI(GLFWwindow *window);
  ~DebugUI();

  DebugUI(const DebugUI &) = delete;
  DebugUI &operator=(const DebugUI &) = delete;

  /// Builds the panel for this frame.
  void build(const DebugUIContext &context);

  /// Draws the panel on the current framebuffer.
  void render();

  /// Enables or disables mouse interaction with the panel.
  void setMouseEnabled(bool enabled);

  void toggleVisible() { mVisible = !mVisible; }

  /// True when ImGui is using the keyboard (a text field has focus).
  [[nodiscard]] bool wantsKeyboard() const;

private:
  bool mVisible = true;

  static void buildSunSection(Scene &scene);
  static void buildShadowSection(RendererSettings &settings);
  static void buildLocalLightSection(const DebugUIContext &context);
  static void buildCameraSection(Camera &camera);
};
