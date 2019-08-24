#include "model.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <iostream>

Mesh::~Mesh()
{
  glDeleteTextures(1, &texture_);
  glDeleteVertexArrays(1, &vao_);
}

Mesh& Mesh::operator=(Mesh&& rhs)
{
  std::swap(vao_, rhs.vao_);
  std::swap(texture_, rhs.texture_);
  std::swap(indices_count_, rhs.indices_count_);
  return *this;
}

Mesh::Mesh(Mesh&& rhs)
    : vao_{rhs.vao_}, texture_{rhs.texture_}, indices_count_{rhs.indices_count_}
{
  rhs.vao_ = 0;
  rhs.texture_ = 0;
  rhs.indices_count_ = 0;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
           std::string_view texture_file)
    : indices_count_{static_cast<GLsizei>(indices.size())}
{
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizei>(vertices.size() * sizeof(Vertex)),
               vertices.data(), GL_STATIC_DRAW);

  unsigned int ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizei>(indices.size() * sizeof(std::uint32_t)),
               indices.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0);
  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void*>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  texture_ = load_texture(texture_file);
}

void Mesh::render()
{
  glBindVertexArray(vao_);

  // bind textures on corresponding texture units
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);

  glDrawElements(GL_TRIANGLES, indices_count_, GL_UNSIGNED_INT, nullptr);
}
