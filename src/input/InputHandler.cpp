#include "InputHandler.h"

InputHandler::InputHandler()
    : m_LastX(0.0f), m_LastY(0.0f), m_FirstMouse(true) {}

void InputHandler::initialize(int windowWidth, int windowHeight) {
  m_LastX = static_cast<float>(windowWidth) / 2.0f;
  m_LastY = static_cast<float>(windowHeight) / 2.0f;
  m_FirstMouse = true;
}

void InputHandler::processInput(GLFWwindow *window, Camera &camera,
                                float deltaTime) {
  // Handle application control
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  // Handle camera movement
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.processKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.processKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.processKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::handleMouseMovement(double xPos, double yPos,
                                       Camera &camera) {
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

  camera.processMouseMovement(xOffset, yOffset);
}

void InputHandler::handleMouseScroll(double xOffset, double yOffset,
                                     Camera &camera) {
  camera.processMouseScroll(static_cast<float>(yOffset));
}