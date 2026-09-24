#include "DebugUI.h"
#include "../rendering/LightManager.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>

DebugUI::DebugUI(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // no imgui.ini dropped in the working directory

  ImGui::StyleColorsDark();
  ImGui::GetStyle().WindowRounding = 4.0f;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}

DebugUI::~DebugUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void DebugUI::setMouseEnabled(const bool enabled) {
  ImGuiIO &io = ImGui::GetIO();
  if (enabled)
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
  else
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

bool DebugUI::wantsKeyboard() const {
  return ImGui::GetIO().WantCaptureKeyboard;
}

void DebugUI::build(const DebugUIContext &context) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (!mVisible)
    return;

  ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize({330.0f, 0.0f}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Renderer");

  const ImGuiIO &io = ImGui::GetIO();
  ImGui::Text("%.0f fps (%.2f ms), %zu draws", io.Framerate,
              1000.0f / io.Framerate, context.drawCount);
  ImGui::TextDisabled(context.cameraMode ? "Tab: use the panel, F1: hide"
                                         : "Tab: back to the camera, F1: hide");

  auto &settings = context.settings;
  auto &lightManager = LightManager::getInstance();
  auto &lights = lightManager.getLights();

  if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen)) {
    float azimuth = lightManager.getSunAzimuth();
    float elevation = lightManager.getSunElevation();
    // Both sliders must be drawn every frame, hence no short-circuit
    bool moved =
        ImGui::SliderFloat("Azimuth", &azimuth, 0.0f, 360.0f, "%.0f deg");
    moved |=
        ImGui::SliderFloat("Elevation", &elevation, 2.0f, 89.0f, "%.0f deg");
    if (moved)
      lightManager.setSun(azimuth, elevation);

    auto &sun = lights.directionalLight;
    ImGui::ColorEdit3("Diffuse##sun", &sun.diffuse.x);
    ImGui::ColorEdit3("Ambient##sun", &sun.ambient.x);
  }

  if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Enabled", &settings.shadowsEnabled);
    ImGui::SameLine();
    ImGui::Checkbox("PCF", &settings.shadowPcf);
    ImGui::SameLine();
    ImGui::Checkbox("Show cascades", &settings.showCascades);
    ImGui::SliderFloat("Split lambda", &settings.cascadeSplitLambda, 0.0f,
                       1.0f);
    ImGui::SliderFloat("Constant bias", &settings.shadowBiasConstant, 0.0f,
                       10.0f, "%.1f texels");
    ImGui::SliderFloat("Slope bias", &settings.shadowBiasSlope, 0.0f, 10.0f,
                       "%.1f texels");
    ImGui::SliderFloat("Normal offset", &settings.shadowNormalOffset, 0.0f,
                       5.0f);
  }

  if (ImGui::CollapsingHeader("Local lights")) {
    bool enabled = lightManager.areLocalLightsEnabled();
    if (ImGui::Checkbox("Enabled##local", &enabled))
      lightManager.setLocalLightsEnabled(enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Show markers", &settings.showLightGizmos);

    for (size_t i = 0; i < lights.pointLights.size(); ++i) {
      auto &light = lights.pointLights[i];
      ImGui::PushID(static_cast<int>(i));
      if (ImGui::TreeNode("Point light", "Point light %zu", i)) {
        ImGui::DragFloat3("Position", &light.position.x, 0.05f);
        ImGui::ColorEdit3("Diffuse", &light.diffuse.x);
        ImGui::SliderFloat("Linear", &light.linear, 0.0f, 1.0f);
        ImGui::SliderFloat("Quadratic", &light.quadratic, 0.0f, 1.0f);
        ImGui::TreePop();
      }
      ImGui::PopID();
    }

    if (ImGui::TreeNode("Spot light")) {
      auto &spot = lights.spotLight;
      ImGui::DragFloat3("Position", &spot.position.x, 0.05f);
      if (ImGui::DragFloat3("Direction", &spot.direction.x, 0.01f, -1.0f,
                            1.0f) &&
          glm::length(spot.direction) < 1e-3f)
        spot.direction = {0.0f, -1.0f, 0.0f};
      ImGui::ColorEdit3("Diffuse", &spot.diffuse.x);
      ImGui::TreePop();
    }
  }

  if (ImGui::CollapsingHeader("Camera")) {
    auto &camera = context.camera;
    ImGui::Text("Position %.1f %.1f %.1f", camera.mPosition.x,
                camera.mPosition.y, camera.mPosition.z);
    ImGui::Text("Yaw %.0f, pitch %.0f", camera.mYaw, camera.mPitch);
    ImGui::SliderFloat("Speed", &camera.mMovementSpeed, 0.5f, 50.0f);
    ImGui::SliderFloat("Field of view", &camera.mZoom, 10.0f, 90.0f, "%.0f");
  }

  if (ImGui::Button("Screenshot (F12)"))
    context.screenshotRequested = true;

  ImGui::End();
}

void DebugUI::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
