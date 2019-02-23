#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

class Camera {
public:
  enum class Movement { forward, backward, left, right };

  // Constructor with vectors
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
                  float pitch = PITCH)
      : front_(glm::vec3(0.0f, 0.0f, -1.0f)), speed_(SPEED),
        MouseSensitivity(SENSITIVITY), zoom_(ZOOM)
  {
    position_ = position;
    world_up_ = up;
    yam_ = yaw;
    pitch_ = pitch;
    updateCameraVectors();
  }
  // Constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch)
      : front_(glm::vec3(0.0f, 0.0f, -1.0f)), speed_(SPEED),
        MouseSensitivity(SENSITIVITY), zoom_(ZOOM)
  {
    position_ = glm::vec3(posX, posY, posZ);
    world_up_ = glm::vec3(upX, upY, upZ);
    yam_ = yaw;
    pitch_ = pitch;
    updateCameraVectors();
  }

  // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 viewMatrix()
  {
    return glm::lookAt(position_, position_ + front_, up_);
  }

  // Processes input received from any keyboard-like input system. Accepts input
  // parameter in the form of camera defined ENUM (to abstract it from windowing
  // systems)
  void move(Movement direction, float deltaTime)
  {
    float dx = speed_ * deltaTime;
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

  // Processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void mouse_movement(float xoffset, float yoffset,
                      GLboolean constrainPitch = true)
  {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

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
    updateCameraVectors();
  }

  // Processes input received from a mouse scroll-wheel event. Only requires
  // input on the vertical wheel-axis
  void mouse_scroll(float yoffset)
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

  float zoom()
  {
    return zoom_;
  }

private:
  // Camera Attributes
  glm::vec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  glm::vec3 world_up_;
  // Euler Angles
  float yam_;
  float pitch_;
  // Camera options
  float speed_;
  float MouseSensitivity;
  float zoom_;

  // Calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors()
  {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yam_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yam_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    right_ = glm::normalize(glm::cross(
        front_, world_up_)); // Normalize the vectors, because their length gets
                             // closer to 0 the more you look up or down which
                             // results in slower movement.
    up_ = glm::normalize(glm::cross(right_, front_));
  }
};
#endif // CAMERA_HPP
