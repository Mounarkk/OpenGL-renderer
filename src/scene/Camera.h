#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

/// Movement directions, kept independent from the windowing library.
enum class CameraMovement { Forward, Backward, Left, Right, Up, Down };

/**
 * Free-fly camera driven by yaw and pitch (Euler angles, in degrees).
 *
 * Pitch is clamped to +/-89 degrees so the view never flips over, which is
 * enough to avoid the gimbal lock issues of Euler angles for an FPS camera.
 * `mZoom` is the vertical field of view in degrees.
 */
class Camera {
public:
  glm::vec3 mPosition;
  glm::vec3 mFront{0.0f, 0.0f, -1.0f};
  glm::vec3 mUp{0.0f, 1.0f, 0.0f};
  glm::vec3 mRight{1.0f, 0.0f, 0.0f};
  glm::vec3 mWorldUp;

  float mYaw;
  float mPitch;

  float mMovementSpeed = 2.5f;
  float mMouseSensitivity = 0.1f;
  float mZoom = 45.0f;

  /**
   * @param position Initial position in world space
   * @param up World up axis
   * @param yaw Initial yaw in degrees, -90 looks down -Z
   * @param pitch Initial pitch in degrees
   */
  explicit Camera(glm::vec3 position = glm::vec3(0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                  float yaw = -90.0f, float pitch = 0.0f);

  [[nodiscard]] glm::mat4 getViewMatrix() const;

  /// Moves along the camera axes, scaled by `deltaTime` seconds.
  void processKeyboard(CameraMovement direction, float deltaTime);

  /// Turns the camera by a mouse offset in pixels.
  void processMouseMovement(float xOffset, float yOffset,
                            bool constrainPitch = true);

  /// Zooms by narrowing or widening the field of view.
  void processMouseScroll(float yOffset);

private:
  /// Rebuilds the front, right and up vectors from yaw and pitch.
  void updateCameraVectors();
};
