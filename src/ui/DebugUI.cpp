#include "DebugUI.h"
#include "../rendering/ShaderInterface.h"
#include "../scene/Entity.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <vector>

DebugUI::DebugUI(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // no imgui.ini dropped in the working directory

  ImGui::StyleColorsDark();
  ImGui::GetStyle().WindowRounding = 4.0f;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450 core");
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
  ImGui::SetNextWindowSize({340.0f, 0.0f}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Renderer");

  const ImGuiIO &io = ImGui::GetIO();
  const RenderStats &stats = context.stats;
  ImGui::Text("%.0f fps (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
  ImGui::Text("Meshes: %zu drawn, %zu in shadows, %zu total",
              stats.drawnLighting, stats.drawnShadow, stats.submitted);
  ImGui::Text("Lights: %d point, %d spot", stats.pointLights, stats.spotLights);
  ImGui::TextDisabled(context.cameraMode ? "Tab: use the panel, F1: hide"
                                         : "Tab: back to the camera, F1: hide");

  buildSunSection(context.scene);
  buildShadowSection(context.settings);
  buildLocalLightSection(context);
  buildCameraSection(context.camera);

  if (ImGui::CollapsingHeader("Performance")) {
    ImGui::Checkbox("Frustum culling", &context.settings.frustumCulling);
  }

  if (ImGui::Button("Screenshot (F12)"))
    context.screenshotRequested = true;

  ImGui::End();
}

void DebugUI::buildSunSection(Scene &scene) {
  if (!ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen))
    return;

  const auto view = scene.getAll<DirectionalLight>();
  if (view.begin() == view.end()) {
    ImGui::TextDisabled("No directional light in the scene");
    return;
  }

  auto &sun = view.get<DirectionalLight>(*view.begin());
  ImGui::SliderFloat("Azimuth", &sun.azimuth, 0.0f, 360.0f, "%.0f deg");
  ImGui::SliderFloat("Elevation", &sun.elevation, 2.0f, 89.0f, "%.0f deg");
  ImGui::ColorEdit3("Color##sun", &sun.color.x);
  ImGui::SliderFloat("Intensity##sun", &sun.intensity, 0.0f, 5.0f);
  ImGui::ColorEdit3("Ambient", &sun.ambient.x);
}

void DebugUI::buildShadowSection(RendererSettings &settings) {
  if (!ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen))
    return;

  ImGui::Checkbox("Enabled", &settings.shadowsEnabled);
  ImGui::SameLine();
  ImGui::Checkbox("PCF", &settings.shadowPcf);
  ImGui::SameLine();
  ImGui::Checkbox("Show cascades", &settings.showCascades);
  ImGui::SliderFloat("Split lambda", &settings.cascadeSplitLambda, 0.0f, 1.0f);
  ImGui::SliderFloat("Constant bias", &settings.shadowBiasConstant, 0.0f, 10.0f,
                     "%.1f texels");
  ImGui::SliderFloat("Slope bias", &settings.shadowBiasSlope, 0.0f, 10.0f,
                     "%.1f texels");
  ImGui::SliderFloat("Normal offset", &settings.shadowNormalOffset, 0.0f, 5.0f);
}

void DebugUI::buildLocalLightSection(const DebugUIContext &context) {
  if (!ImGui::CollapsingHeader("Local lights"))
    return;

  auto &settings = context.settings;
  auto &scene = context.scene;
  auto &registry = scene.getRegistry();

  ImGui::Checkbox("Enabled##local", &settings.localLightsEnabled);
  ImGui::SameLine();
  ImGui::Checkbox("Show markers", &settings.showLightGizmos);

  // Entities are destroyed after the loops, not while iterating the views
  std::vector<entt::entity> toDestroy;

  for (const auto entity : scene.getAll<Tag, Transform, PointLight>()) {
    auto [tag, transform, light] =
        registry.get<Tag, Transform, PointLight>(entity);
    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    if (ImGui::TreeNode(tag.name.c_str())) {
      ImGui::DragFloat3("Position", &transform.position.x, 0.05f);
      ImGui::ColorEdit3("Color", &light.color.x);
      ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 50.0f);
      ImGui::SliderFloat("Range", &light.range, 0.1f, 50.0f);
      if (ImGui::Button("Remove"))
        toDestroy.push_back(entity);
      ImGui::TreePop();
    }
    ImGui::PopID();
  }

  for (const auto entity : scene.getAll<Tag, Transform, SpotLight>()) {
    auto [tag, transform, light] =
        registry.get<Tag, Transform, SpotLight>(entity);
    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    if (ImGui::TreeNode(tag.name.c_str())) {
      ImGui::DragFloat3("Position", &transform.position.x, 0.05f);
      glm::vec2 angles = glm::degrees(glm::vec2(transform.rotation));
      if (ImGui::DragFloat2("Pitch, yaw", &angles.x, 0.5f)) {
        transform.rotation.x = glm::radians(angles.x);
        transform.rotation.y = glm::radians(angles.y);
      }
      ImGui::ColorEdit3("Color", &light.color.x);
      ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 50.0f);
      ImGui::SliderFloat("Range", &light.range, 0.1f, 50.0f);
      ImGui::SliderFloat("Inner angle", &light.innerAngle, 1.0f,
                         light.outerAngle, "%.1f deg");
      ImGui::SliderFloat("Outer angle", &light.outerAngle, light.innerAngle,
                         80.0f, "%.1f deg");
      if (ImGui::Button("Remove"))
        toDestroy.push_back(entity);
      ImGui::TreePop();
    }
    ImGui::PopID();
  }

  for (const auto entity : toDestroy)
    registry.destroy(entity);

  // New lights appear a few meters in front of the camera
  const glm::vec3 spawn =
      context.camera.mPosition + context.camera.mFront * 3.0f;
  if (ImGui::Button("Add point light") &&
      context.stats.pointLights < ShaderInterface::kMaxPointLights) {
    static int created = 0;
    Entity light =
        scene.createEntity("New point light " + std::to_string(created++));
    Transform transform;
    transform.position = spawn;
    light.addComponent<Transform>(transform);
    light.addComponent<PointLight>();
  }
  ImGui::SameLine();
  if (ImGui::Button("Add spot light") &&
      context.stats.spotLights < ShaderInterface::kMaxSpotLights) {
    static int created = 0;
    Entity light =
        scene.createEntity("New spot light " + std::to_string(created++));
    Transform transform;
    transform.position = spawn + glm::vec3(0.0f, 2.0f, 0.0f);
    transform.rotation.x = glm::radians(-90.0f); // pointing down
    light.addComponent<Transform>(transform);
    light.addComponent<SpotLight>();
  }
}

void DebugUI::buildCameraSection(Camera &camera) {
  if (!ImGui::CollapsingHeader("Camera"))
    return;

  ImGui::Text("Position %.1f %.1f %.1f", camera.mPosition.x, camera.mPosition.y,
              camera.mPosition.z);
  ImGui::Text("Yaw %.0f, pitch %.0f", camera.mYaw, camera.mPitch);
  ImGui::SliderFloat("Speed", &camera.mMovementSpeed, 0.5f, 50.0f);
  ImGui::SliderFloat("Field of view", &camera.mZoom, 10.0f, 90.0f, "%.0f");
}

void DebugUI::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
