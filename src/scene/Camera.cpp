#include "Camera.h"

#include <algorithm>
#include <cmath>

Camera::Camera(const glm::vec3 position, const glm::vec3 up, const float yaw,
               const float pitch)
    : mPosition(position), mWorldUp(up), mYaw(yaw), mPitch(pitch) {
  updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

void Camera::processKeyboard(const CameraMovement direction,
                             const float deltaTime) {
  const float velocity = mMovementSpeed * deltaTime;
  switch (direction) {
  case CameraMovement::Forward:
    mPosition += mFront * velocity;
    break;
  case CameraMovement::Backward:
    mPosition -= mFront * velocity;
    break;
  case CameraMovement::Left:
    mPosition -= mRight * velocity;
    break;
  case CameraMovement::Right:
    mPosition += mRight * velocity;
    break;
  case CameraMovement::Up:
    mPosition += mWorldUp * velocity;
    break;
  case CameraMovement::Down:
    mPosition -= mWorldUp * velocity;
    break;
  }
}

void Camera::processMouseMovement(float xOffset, float yOffset,
                                  const bool constrainPitch) {
  mYaw += xOffset * mMouseSensitivity;
  mPitch += yOffset * mMouseSensitivity;

  if (constrainPitch)
    mPitch = std::clamp(mPitch, -89.0f, 89.0f);

  updateCameraVectors();
}

void Camera::processMouseScroll(const float yOffset) {
  mZoom = std::clamp(mZoom - yOffset, 1.0f, 90.0f);
}

void Camera::updateCameraVectors() {
  const float yaw = glm::radians(mYaw);
  const float pitch = glm::radians(mPitch);
  mFront =
      glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch), std::sin(pitch),
                               std::sin(yaw) * std::cos(pitch)));
  // Normalized because the cross product shrinks as pitch approaches +/-90,
  // which would slow down strafing when looking up or down
  mRight = glm::normalize(glm::cross(mFront, mWorldUp));
  mUp = glm::normalize(glm::cross(mRight, mFront));
}
