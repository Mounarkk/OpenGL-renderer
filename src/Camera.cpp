#include "Camera.h"

// constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : mFront(glm::vec3(0.0f, 0.0f, -1.0f)), mMovementSpeed(SPEED),
      mMouseSensitivity(SENSITIVITY), mZoom(ZOOM) {
  mPosition = position;
  mWorldUp = up;
  mYaw = yaw;
  mPitch = pitch;
  updateCameraVectors();
}

// constructor with scalar values
Camera::Camera(float posX, float posY, float posZ, float upX, float upY,
               float upZ, float yaw, float pitch)
    : mFront(glm::vec3(0.0f, 0.0f, -1.0f)), mMovementSpeed(SPEED),
      mMouseSensitivity(SENSITIVITY), mZoom(ZOOM) {
  mPosition = glm::vec3(posX, posY, posZ);
  mWorldUp = glm::vec3(upX, upY, upZ);
  mYaw = yaw;
  mPitch = pitch;
  updateCameraVectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(mPosition, this->mPosition + this->mFront, this->mUp);
}

// processes input received from any keyboard-like input system. Accepts input
// parameter in the form of camera defined ENUM (to abstract it from windowing
// systems)
void Camera::processKeyboard(const ProcessKeyboard direction,
                             const float deltaTime) {
  const float velocity = mMovementSpeed * deltaTime;
  if (direction == FORWARD)
    mPosition += mFront * velocity;
  if (direction == BACKWARD)
    mPosition -= mFront * velocity;
  if (direction == LEFT)
    mPosition -= mRight * velocity;
  if (direction == RIGHT)
    mPosition += mRight * velocity;
}

// processes input received from a mouse input system. Expects the offset value
// in both the x and y direction.
void Camera::processMouseMovement(float xOffset, float yOffset,
                                  const GLboolean constrainPitch) {
  xOffset *= mMouseSensitivity;
  yOffset *= mMouseSensitivity;

  mYaw += xOffset;
  mPitch += yOffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrainPitch) {
    if (mPitch > 89.0f)
      mPitch = 89.0f;
    if (mPitch < -89.0f)
      mPitch = -89.0f;
  }

  // update Front, Right and Up Vectors using the updated Euler angles
  updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input
// on the vertical wheel-axis
void Camera::processMouseScroll(const float yOffset) {
  mZoom -= static_cast<float>(yOffset);
  if (mZoom < 1.0f)
    mZoom = 1.0f;
  if (mZoom > 45.0f)
    mZoom = 45.0f;
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors() {
  // calculate the new Front vector
  glm::vec3 front;
  front.x =
      static_cast<float>(cos(glm::radians(mYaw)) * cos(glm::radians(mPitch)));
  front.y = static_cast<float>(sin(glm::radians(mPitch)));
  front.z =
      static_cast<float>(sin(glm::radians(mYaw)) * cos(glm::radians(mPitch)));
  mFront = glm::normalize(front);
  // also re-calculate the Right and Up vector
  mRight = glm::normalize(glm::cross(
      mFront, mWorldUp)); // normalize the vectors, because their length gets
                          // closer to 0 the more you look up or down which
                          // results in slower movement.
  mUp = glm::normalize(glm::cross(mRight, mFront));
}