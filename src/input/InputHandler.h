#pragma once
#include "../scene/Camera.h"

#include <GLFW/glfw3.h>

/**
 * Translates GLFW input into camera movement.
 *
 * Held keys are polled every frame (WASD, Space, Left Ctrl, Left Shift to go
 * faster). Mouse motion and scroll arrive through the window callbacks.
 */
class InputHandler {
public:
  /// Polls held keys and moves the camera accordingly.
  void processInput(GLFWwindow *window, Camera &camera, float deltaTime);

  /// Turns the camera from an absolute cursor position.
  void handleMouseMovement(double xPos, double yPos, Camera &camera);

  void handleMouseScroll(double yOffset, Camera &camera);

  /// Forgets the last cursor position, to call when the camera takes the
  /// mouse back so that it does not jump.
  void resetMouse() { m_FirstMouse = true; }

private:
  float m_LastX = 0.0f;
  float m_LastY = 0.0f;
  // The first cursor event only records the position, otherwise the camera
  // would jump by the distance between the cursor and the window center.
  bool m_FirstMouse = true;
};
