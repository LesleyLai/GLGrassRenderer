#include "model.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

Model::Model(std::vector<Vertex> vertices, std::string_view texture_file)
    : vertices_{std::move(vertices)}
{
  unsigned int vbo;
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizei>(vertices_.size() * sizeof(Vertex)),
               vertices_.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0);
  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void*>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // glDeleteBuffers(1, &vbo);

  texture_ = load_texture(texture_file);
}

// Model::~Model()
//{
//  glDeleteVertexArrays(1, &vao_);
//}

void Model::render()
{
  glBindVertexArray(vao_);

  // bind textures on corresponding texture units
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
