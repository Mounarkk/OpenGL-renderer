#pragma once
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum ProcessKeyboard { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
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

  // constructor with vectors
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
                  float pitch = PITCH);
  // constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch);

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  [[nodiscard]] glm::mat4 getViewMatrix() const;

  // processes input received from any keyboard-like input system. Accepts input
  // parameter in the form of camera defined ENUM (to abstract it from windowing
  // systems)
  void processKeyboard(ProcessKeyboard direction, float deltaTime);

  // processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void processMouseMovement(float xOffset, float yOffset,
                            GLboolean constrainPitch = true);

  // processes input received from a mouse scroll-wheel event. Only requires
  // input on the vertical wheel-axis
  void processMouseScroll(float yOffset);

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors();
};

#endif // CAMERA_H
