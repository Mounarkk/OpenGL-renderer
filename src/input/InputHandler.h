#pragma once
#include "../scene/Camera.h"
#include <GLFW/glfw3.h>

/**
 * Handles all input processing for the application.
 *
 * The InputHandler class centralizes all input processing logic, including
 * keyboard input for camera movement and application control, mouse movement
 * for camera look controls, and mouse scroll for zoom. This separation allows
 * the Application class to focus on coordination while keeping input logic
 * organized and testable.
 *
 * Key responsibilities:
 * - Process keyboard input for camera movement and application control
 * - Handle mouse movement for first-person camera controls
 * - Process mouse scroll for camera zoom
 * - Maintain input state (mouse position, first mouse flag)
 */
class InputHandler {
public:
  /**
   * Constructs an InputHandler with default settings.
   * Initializes mouse tracking state for proper camera controls.
   */
  InputHandler();

  /**
   * Processes all keyboard input for the current frame.
   * Handles camera movement (WASD) and application control (ESC).
   * @param window The GLFW window to check for input
   * @param camera Camera to update based on movement input
   * @param deltaTime Time elapsed since last frame for frame-rate independent
   * movement
   */
  void processInput(GLFWwindow *window, Camera &camera, float deltaTime);

  /**
   * Handles mouse movement for camera look controls.
   * Updates camera orientation based on mouse movement offsets.
   * @param xPos Current mouse X position in screen coordinates
   * @param yPos Current mouse Y position in screen coordinates
   * @param camera Camera to update based on mouse movement
   */
  void handleMouseMovement(double xPos, double yPos, Camera &camera);

  /**
   * Handles mouse scroll for camera zoom controls.
   * @param xOffset Horizontal scroll offset (usually unused)
   * @param yOffset Vertical scroll offset for zoom control
   * @param camera Camera to update based on scroll input
   */
  void handleMouseScroll(double xOffset, double yOffset, Camera &camera);

  /**
   * Initializes input handler with window dimensions for proper mouse tracking.
   * @param windowWidth Initial window width for mouse position calculation
   * @param windowHeight Initial window height for mouse position calculation
   */
  void initialize(int windowWidth, int windowHeight);

private:
  // Mouse tracking state
  float m_LastX;     ///< Last recorded mouse X position
  float m_LastY;     ///< Last recorded mouse Y position
  bool m_FirstMouse; ///< Flag to handle first mouse movement properly
};