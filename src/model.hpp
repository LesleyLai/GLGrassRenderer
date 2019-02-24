#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string_view>
#include <vector>

#include "texture.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec2 tex_coord;
};

class Model {
public:
  explicit Model(std::vector<Vertex> vertices, std::string_view texture_file);

  void render();

private:
  std::vector<Vertex> vertices_;
  unsigned int vao_;
  unsigned int texture_;
};

#endif // MODEL_HPP
