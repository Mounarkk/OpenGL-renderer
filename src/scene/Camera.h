#pragma once
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

/**
 * Camera movement directions for keyboard input.
 * Used as abstraction to stay away from window-system specific input methods.
 */
enum ProcessKeyboard { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera configuration values
constexpr float YAW = -90.0f;        // Default yaw angle (looking down negative Z-axis)
constexpr float PITCH = 0.0f;        // Default pitch angle (level horizon)
constexpr float SPEED = 2.5f;        // Default movement speed (units per second)
constexpr float SENSITIVITY = 0.1f;  // Default mouse sensitivity
constexpr float ZOOM = 45.0f;        // Default field of view in degrees

/**
 * First-person camera implementation using Euler angles.
 * 
 * This camera class provides a complete first-person camera system suitable
 * for 3D applications. It handles mouse look controls, keyboard movement,
 * and generates the appropriate view matrices for rendering. The camera uses
 * Euler angles (yaw/pitch) for rotation, which is intuitive for FPS-style
 * controls but avoids gimbal lock by constraining pitch.
 * 
 * Key features:
 * - First-person mouse look controls with pitch constraints
 * - WASD-style keyboard movement in world space
 * - Mouse scroll zoom (field of view adjustment)
 * - Automatic vector calculations for OpenGL view matrices
 */
class Camera {
public:
  // camera Attributes
  glm::vec3 mPosition;
  glm::vec3 mFront;
  glm::vec3 mUp;
  glm::vec3 mRight;
  glm::vec3 mWorldUp;
  // euler Angles
  float mYaw;
  float mPitch;
  // camera options
  float mMovementSpeed;
  float mMouseSensitivity;
  float mZoom;

  /**
   * Constructs a camera with vector parameters.
   * @param position Initial camera position in world space
   * @param up World up vector (usually (0,1,0) for Y-up coordinate system)
   * @param yaw Initial yaw angle in degrees (rotation around Y-axis)
   * @param pitch Initial pitch angle in degrees (rotation around X-axis)
   */
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
                  float pitch = PITCH);
  
  /**
   * Constructs a camera with scalar parameters.
   * @param posX Initial X position
   * @param posY Initial Y position  
   * @param posZ Initial Z position
   * @param upX World up vector X component
   * @param upY World up vector Y component
   * @param upZ World up vector Z component
   * @param yaw Initial yaw angle in degrees
   * @param pitch Initial pitch angle in degrees
   */
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch);

  /**
   * Calculates and returns the view matrix for rendering.
   * The view matrix transforms world coordinates to camera space.
   * @return 4x4 view matrix calculated from current camera state
   */
  [[nodiscard]] glm::mat4 getViewMatrix() const;

  /**
   * Processes keyboard input for camera movement.
   * Moves the camera in the appropriate direction based on current orientation.
   * @param direction Movement direction (FORWARD, BACKWARD, LEFT, RIGHT)
   * @param deltaTime Time elapsed since last frame (for frame-rate independent movement)
   */
  void processKeyboard(ProcessKeyboard direction, float deltaTime);

  /**
   * Processes mouse movement for camera look controls.
   * Updates yaw and pitch based on mouse movement offsets.
   * @param xOffset Mouse movement in X direction (horizontal)
   * @param yOffset Mouse movement in Y direction (vertical)
   * @param constrainPitch Whether to limit pitch to prevent camera flipping (default: true)
   */
  void processMouseMovement(float xOffset, float yOffset,
                            GLboolean constrainPitch = true);

  /**
   * Processes mouse scroll input for zoom control.
   * Adjusts the field of view to create a zoom effect.
   * @param yOffset Scroll wheel offset (positive = zoom in, negative = zoom out)
   */
  void processMouseScroll(float yOffset);

private:
  /**
   * Recalculates camera direction vectors from current Euler angles.
   * Updates mFront, mRight, and mUp vectors based on current yaw and pitch.
   * Called automatically when angles change.
   */
  void updateCameraVectors();
};

