#include "camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front_(glm::vec3(0.0f, 0.0f, -1.0f)), speed_(initial_speed),
      mouse_sensitivity_(init_sensitivity), zoom_(init_zoom)
{
  position_ = position;
  world_up_ = up;
  yam_ = yaw;
  pitch_ = pitch;
  update_camera_vectors();
}

void Camera::move(Camera::Movement direction,
                  std::chrono::duration<float, std::milli> delta_time)
{
  float dx = speed_ * delta_time.count() / 1000;
  switch (direction) {
  case Camera::Movement::forward:
    position_ += front_ * dx;
    break;
  case Camera::Movement::backward:
    position_ -= front_ * dx;
    break;
  case Camera::Movement::left:
    position_ -= right_ * dx;
    break;
  case Camera::Movement::right:
    position_ += right_ * dx;
    break;
  }
}

void Camera::mouse_movement(float xoffset, float yoffset,
                            GLboolean constrainPitch)
{
  xoffset *= mouse_sensitivity_;
  yoffset *= mouse_sensitivity_;

  yam_ += xoffset;
  pitch_ += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrainPitch) {
    if (pitch_ > 89.0f) {
      pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f) {
      pitch_ = -89.0f;
    }
  }

  // Update Front, Right and Up Vectors using the updated Euler angles
  update_camera_vectors();
}

void Camera::mouse_scroll(float yoffset)
{
  if (zoom_ >= 1.0f && zoom_ <= 45.0f) {
    zoom_ -= yoffset;
  }
  if (zoom_ <= 1.0f) {
    zoom_ = 1.0f;
  }
  if (zoom_ >= 45.0f) {
    zoom_ = 45.0f;
  }
}

void Camera::update_camera_vectors()
{
  // Calculate the new Front vector
  glm::vec3 front;
  front.x = std::cos(glm::radians(yam_)) * std::cos(glm::radians(pitch_));
  front.y = std::sin(glm::radians(pitch_));
  front.z = std::sin(glm::radians(yam_)) * std::cos(glm::radians(pitch_));
  front_ = glm::normalize(front);
  // Also re-calculate the Right and Up vector
  right_ = glm::normalize(glm::cross(
      front_, world_up_)); // Normalize the vectors, because their length gets
  // closer to 0 the more you look up or down which
  // results in slower movement.
  up_ = glm::normalize(glm::cross(right_, front_));
}
