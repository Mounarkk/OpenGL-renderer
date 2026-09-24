#include "InputHandler.h"

namespace {
constexpr float kSprintMultiplier = 4.0f;
}

void InputHandler::processInput(GLFWwindow *window, Camera &camera,
                                float deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    deltaTime *= kSprintMultiplier;

  const auto held = [window](const int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
  };

  if (held(GLFW_KEY_W))
    camera.processKeyboard(CameraMovement::Forward, deltaTime);
  if (held(GLFW_KEY_S))
    camera.processKeyboard(CameraMovement::Backward, deltaTime);
  if (held(GLFW_KEY_A))
    camera.processKeyboard(CameraMovement::Left, deltaTime);
  if (held(GLFW_KEY_D))
    camera.processKeyboard(CameraMovement::Right, deltaTime);
  if (held(GLFW_KEY_SPACE))
    camera.processKeyboard(CameraMovement::Up, deltaTime);
  if (held(GLFW_KEY_LEFT_CONTROL))
    camera.processKeyboard(CameraMovement::Down, deltaTime);
}

void InputHandler::handleMouseMovement(const double xPos, const double yPos,
                                       Camera &camera) {
  const auto x = static_cast<float>(xPos);
  const auto y = static_cast<float>(yPos);

  if (m_FirstMouse) {
    m_LastX = x;
    m_LastY = y;
    m_FirstMouse = false;
  }

  // Screen Y grows downwards, pitch grows upwards
  camera.processMouseMovement(x - m_LastX, m_LastY - y);
  m_LastX = x;
  m_LastY = y;
}

void InputHandler::handleMouseScroll(const double yOffset, Camera &camera) {
  camera.processMouseScroll(static_cast<float>(yOffset));
}
