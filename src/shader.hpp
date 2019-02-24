#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/format.h>

class ShaderProgram;

/**
 * @ingroup opengl
 * @brief The Shader class
 */
struct Shader {
public:
  /// OpenGL_shader type
  enum class Type : GLenum {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    TessControl = GL_TESS_CONTROL_SHADER,
    TessEval = GL_TESS_EVALUATION_SHADER,
    Geometry = GL_GEOMETRY_SHADER,
    Compute = GL_COMPUTE_SHADER
  };

  Shader(const char* source, Type type);
  ~Shader();

  Shader(const Shader& other) = delete;
  Shader& operator=(const Shader& other) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

private:
  Type type_;
  unsigned int id_;

  friend ShaderProgram;
};

// Read whole file into a string
[[nodiscard]] inline std::string readFile(std::string_view path)
{
  std::ifstream file{path.data()};

  if (!file.is_open()) {
    fmt::print(stderr, "Cannot open file {}\n", path);
    std::fflush(stdout);
  }

  std::stringstream ss;
  // read file's buffer contents into streams
  ss << file.rdbuf();

  return ss.str();
}

class ShaderProgram {
public:
  ShaderProgram() = default;
  explicit ShaderProgram(const std::vector<Shader>& shaders);

  void use() const
  {
    glUseProgram(id_);
  }

  void setBool(const std::string& name, bool value) const
  {
    glUniform1i(glGetUniformLocation(id_, name.c_str()),
                static_cast<int>(value));
  }
  void setInt(const std::string& name, int value) const
  {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
  }
  void setFloat(const std::string& name, float value) const
  {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
  }
  void setVec2(const std::string& name, const glm::vec2& value) const
  {
    glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
  }
  void setVec2(const std::string& name, float x, float y) const
  {
    glUniform2f(glGetUniformLocation(id_, name.c_str()), x, y);
  }
  void setVec3(const std::string& name, const glm::vec3& value) const
  {
    glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
  }
  void setVec3(const std::string& name, float x, float y, float z) const
  {
    glUniform3f(glGetUniformLocation(id_, name.c_str()), x, y, z);
  }
  void setVec4(const std::string& name, const glm::vec4& value) const
  {
    glUniform4fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
  }
  void setVec4(const std::string& name, float x, float y, float z,
               float w) const
  {
    glUniform4f(glGetUniformLocation(id_, name.c_str()), x, y, z, w);
  }
  void setMat2(const std::string& name, const glm::mat2& mat) const
  {
    glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void setMat3(const std::string& name, const glm::mat3& mat) const
  {
    glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void setMat4(const std::string& name, const glm::mat4& mat) const
  {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }

private:
  unsigned int id_;
};

class ShaderBuilder {
public:
  ShaderBuilder() = default;

  ShaderBuilder& load(std::string_view filename, Shader::Type type)
  {
    const std::string src = readFile(filename);
    shaders_.emplace_back(src.c_str(), type);
    return *this;
  }

  [[nodiscard]] ShaderProgram build() const
  {
    return ShaderProgram{shaders_};
  }

private:
  std::vector<Shader> shaders_;
};

#endif // SHADER_HPP
