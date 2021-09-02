#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>

class Camera {
public:
  // Default camera values
  static constexpr float init_yaw = -90.0f;
  static constexpr float init_pitch = 0.0f;
  static constexpr float initial_speed = 2.5f;
  static constexpr float init_sensitivity = 0.1f;
  static constexpr float init_zoom = 45.0f;

  enum class Movement { forward, backward, left, right };

  // Constructor with vectors
  explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                  float yaw = init_yaw, float pitch = init_pitch);

  [[nodiscard]] glm::mat4 view_matrix() const
  {
    return glm::lookAt(position_, position_ + front_, up_);
  }

  void move(Movement direction,
            std::chrono::duration<float, std::milli> delta_time);

  // Processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void mouse_movement(float xoffset, float yoffset,
                      GLboolean constrainPitch = true);

  void mouse_scroll(float yoffset);

  [[nodiscard]] float zoom() const
  {
    return zoom_;
  }

  [[nodiscard]] float speed() const
  {
    return speed_;
  }

  void set_speed(float speed)
  {
    speed_ = speed;
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
  float mouse_sensitivity_;
  float zoom_;

  void update_camera_vectors();
};
#endif // CAMERA_HPP
