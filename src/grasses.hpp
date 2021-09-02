#ifndef GLGRASSRENDERER_GRASSES_HPP
#define GLGRASSRENDERER_GRASSES_HPP

#include "shader.hpp"

#include <chrono>

class Grasses {
  unsigned int grass_vao_ = 0;
  ShaderProgram grass_shader_{};
  ShaderProgram grass_compute_shader_{};
  GLuint blades_count_ = 0;

public:
  // Wind parameters
  float wind_magnitude = 1.0;
  float wind_wave_length = 1.0;
  float wind_wave_period = 1.0;

  using DeltaDuration = std::chrono::duration<float, std::milli>;

  void init();
  void update(DeltaDuration delta_time);
  void render();
};

#endif // GLGRASSRENDERER_GRASSES_HPP
