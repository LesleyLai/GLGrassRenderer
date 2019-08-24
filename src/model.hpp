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

class Mesh {
public:
  explicit Mesh(std::vector<Vertex> vertices,
                std::vector<std::uint32_t> indices,
                std::string_view texture_file);
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&& rhs);
  Mesh& operator=(Mesh&& rhs);

  void render();

private:
  unsigned int vao_ = 0;
  unsigned int texture_ = 0;
  int indices_count_ = 0;
};

#endif // MODEL_HPP
