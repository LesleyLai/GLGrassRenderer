//
// Created by laisi on 9/2/2021.
//

#include "grasses.hpp"

#include <random>
#include <vector>

#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

// Indirect drawing structure
struct NumBlades {
  std::uint32_t vertexCount = 5;
  std::uint32_t instanceCount = 1;
  std::uint32_t firstVertex = 0;
  std::uint32_t firstInstance = 0;
};

struct Blade {
  glm::vec4 v0; // xyz: Position, w: orientation (in radius)
  glm::vec4 v1; // xyz: Bezier point w: height
  glm::vec4 v2; // xyz: Physical model guide w: width
  glm::vec4 up; // xyz: Up vector w: stiffness coefficient
};

namespace {

std::vector<Blade> generate_blades()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> orientation_dis(0, glm::pi<float>());
  std::uniform_real_distribution<float> height_dis(0.6f, 1.2f);
  std::uniform_real_distribution<float> dis(-1, 1);

  std::vector<Blade> blades;
  for (int i = -200; i < 200; ++i) {
    for (int j = -200; j < 200; ++j) {
      const auto x = static_cast<float>(i) / 10 - 1 + dis(gen) * 0.1f;
      const auto y = static_cast<float>(j) / 10 - 1 + dis(gen) * 0.1f;
      const auto blade_height = height_dis(gen);

      blades.emplace_back(
          glm::vec4(x, 0, y, orientation_dis(gen)),
          glm::vec4(x, blade_height, y, blade_height),
          glm::vec4(x, blade_height, y, 0.1f),
          glm::vec4(0, blade_height, 0, 0.7f + dis(gen) * 0.3f));
    }
  }
  return blades;
}

} // anonymous namespace

void Grasses::init()
{
  const std::vector<Blade> blades = generate_blades();
  blades_count_ = static_cast<GLuint>(blades.size());

  glPatchParameteri(GL_PATCH_VERTICES, 1);

  glGenVertexArrays(1, &grass_vao_);
  glBindVertexArray(grass_vao_);

  unsigned int grass_input_buffer = 0;
  glGenBuffers(1, &grass_input_buffer);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, grass_input_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               static_cast<GLsizei>(blades.size() * sizeof(Blade)),
               blades.data(), GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grass_input_buffer);

  unsigned int grass_output_buffer = 0;
  glGenBuffers(1, &grass_output_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, grass_output_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               static_cast<GLsizei>(blades.size() * sizeof(Blade)), nullptr,
               GL_STREAM_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grass_output_buffer);

  NumBlades numBlades;
  unsigned int grass_indirect_buffer = 0;
  glGenBuffers(1, &grass_indirect_buffer);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grass_indirect_buffer);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(NumBlades), &numBlades,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, grass_output_buffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grass_indirect_buffer);

  // v0 attribute
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Blade),
                        reinterpret_cast<void*>(offsetof(Blade, v0)));
  glEnableVertexAttribArray(0);

  // v1 attribute
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Blade),
                        reinterpret_cast<void*>(offsetof(Blade, v1)));
  glEnableVertexAttribArray(1);

  // v2 attribute
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Blade),
                        reinterpret_cast<void*>(offsetof(Blade, v2)));
  glEnableVertexAttribArray(2);

  // dir attribute
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Blade),
                        reinterpret_cast<void*>(offsetof(Blade, up)));
  glEnableVertexAttribArray(3);

  grass_compute_shader_ =
      ShaderBuilder{}.load("grass.comp.glsl", Shader::Type::Compute).build();
  grass_compute_shader_.use();

  grass_shader_ = ShaderBuilder{}
                      .load("grass.vert.glsl", Shader::Type::Vertex)
                      .load("grass.tesc.glsl", Shader::Type::TessControl)
                      .load("grass.tese.glsl", Shader::Type::TessEval)
                      .load("grass.frag.glsl", Shader::Type::Fragment)
                      .build();
}

void Grasses::update(DeltaDuration delta_time)
{
  grass_compute_shader_.use();
  grass_compute_shader_.setFloat("current_time",
                                 static_cast<float>(glfwGetTime()));
  grass_compute_shader_.setFloat("delta_time", delta_time.count() / 1e3f);
  grass_compute_shader_.setFloat("wind_magnitude", wind_magnitude);
  grass_compute_shader_.setFloat("wind_wave_length", wind_wave_length);
  grass_compute_shader_.setFloat("wind_wave_period", wind_wave_period);

  glDispatchCompute(static_cast<GLuint>(blades_count_), 1, 1);
}

void Grasses::render()
{
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  glBindVertexArray(grass_vao_);
  grass_shader_.use();
  glDrawArraysIndirect(GL_PATCHES, reinterpret_cast<void*>(0));
}