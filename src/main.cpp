#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string_view>
#include <vector>

#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"

#include <fmt/format.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

struct Blade {
  // Position and direction
  glm::vec4 v0;
  // Bezier point and height
  glm::vec4 v1;
  // Physical model guide and width
  glm::vec4 v2;
  // Up vector and stiffness coefficient
  glm::vec4 up;

  Blade(const glm::vec4& iv0, const glm::vec4& iv1, const glm::vec4& iv2,
        const glm::vec4& iup)
      : v0{iv0}, v1{iv1}, v2{iv2}, up{iup}
  {
  }
};

class App {
public:
  App(int width, int height, std::string_view title)
      : width_{width}, height_{height}
  {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width_, height_, title.data(), nullptr, nullptr);
    if (window_ == nullptr) {
      fmt::print(stderr, "Failed to create GLFW window\n");
      glfwTerminate();
      std::exit(1);
    }
    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, mouse_callback);
    glfwSetScrollCallback(window_, scroll_callback);

    glfwSetWindowUserPointer(window_, this);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
      fmt::print(stderr, "Failed to initialize GLAD\n");
      glfwTerminate();
      std::exit(1);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    {
      // clang-format off
      std::vector<Vertex> verts {
        {{-1.0f, 0.0f, -1.0f},  {0.0f, 1.0f}},
        {{1.0f, 0.0f,  1.0f},   {1.0f, 0.0f}},
        {{1.0f, 0.0f, -1.0f},   {1.0f, 1.0f}},
        {{1.0f, 0.0f,  1.0f},   {1.0f, 0.0f}},
        {{-1.0f, 0.0f, -1.0f},  {0.0f, 1.0f}},
        {{-1.0f, 0.0f,  1.0f},  {0.0f, 0.0f}},
      };
      // clang-format on
      land_ = std::make_unique<Model>(verts, "GrassGreenTexture0001.jpg");
      land_shader_ = ShaderBuilder{}
                         .load("land.vert", Shader::Type::Vertex)
                         .load("land.frag", Shader::Type::Fragment)
                         .build();
      land_shader_.use();
      land_shader_.setInt("texture1", 0);
    }

    {
      blades_.emplace_back(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0.4, 0, 1),
                           glm::vec4(0, 0.4, 0, 0.1), glm::vec4(0, 1, 0, 1));

      glPatchParameteri(GL_PATCH_VERTICES, 1);

      unsigned int vbo;
      glGenVertexArrays(1, &grass_vao_);
      glGenBuffers(1, &vbo);
      glBindVertexArray(grass_vao_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER,
                   static_cast<GLsizei>(blades_.size() * sizeof(Blade)),
                   blades_.data(), GL_STATIC_DRAW);

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

      grass_shader_ = ShaderBuilder{}
                          .load("grass.vert", Shader::Type::Vertex)
                          .load("grass.tesc", Shader::Type::TessControl)
                          .load("grass.tese", Shader::Type::TessEval)
                          .load("grass.frag", Shader::Type::Fragment)
                          .build();

      grass_compute_shader_ =
          ShaderBuilder{}.load("grass.comp", Shader::Type::Compute).build();
    }

    glGenBuffers(1, &cameraUniformBuffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBuffer_);
    glBufferData(GL_UNIFORM_BUFFER, 128, nullptr,
                 GL_STATIC_DRAW); // allocate 150 bytes of memory
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void run()
  {
    while (!glfwWindowShouldClose(window_)) {
      float currentFrame = glfwGetTime();
      deltaTime_ = currentFrame - lastFrame;
      lastFrame = currentFrame;

      processInput(window_);

      // render
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // activate shader
      land_shader_.use();

      // camera/view transformation
      glm::mat4 view = camera_.viewMatrix();
      glm::mat4 projection = glm::perspective(glm::radians(camera_.zoom()),
                                              static_cast<float>(width_) /
                                                  static_cast<float>(height_),
                                              0.1f, 100.0f);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformBuffer_);
      glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBuffer_);

      glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &view);
      glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, &projection);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      // render boxes
      // calculate the model matrix for each object and pass it to shader before
      // drawing
      glm::mat4 model = glm::mat4(1.0f); // Identity
      model = glm::scale(model, glm::vec3(2, 2, 2));
      land_shader_.setMat4("model", model);
      land_->render();

      grass_shader_.use();
      grass_shader_.setMat4("model", model);
      glBindVertexArray(grass_vao_);
      glDrawArrays(GL_PATCHES, 0, static_cast<GLsizei>(blades_.size()));

      glfwSwapBuffers(window_);
      glfwPollEvents();
    }
  }

  ~App()
  {
    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  App(const App& app) = delete;
  App& operator=(const App& app) = delete;
  App(App&& app) = delete;
  App& operator=(App&& app) = delete;

  [[nodiscard]] Camera& camera() noexcept
  {
    return camera_;
  }

  [[nodiscard]] float deltaTime() const noexcept
  {
    return deltaTime_;
  }

  [[nodiscard]] int width() const noexcept
  {
    return width_;
  }

  [[nodiscard]] int height() const noexcept
  {
    return height_;
  }

private:
  GLFWwindow* window_;
  int width_;
  int height_;

  std::unique_ptr<Model> land_;
  ShaderProgram land_shader_{};

  unsigned int grass_vao_ = 0;
  ShaderProgram grass_shader_{};
  ShaderProgram grass_compute_shader_{};
  std::vector<Blade> blades_;

  unsigned int cameraUniformBuffer_ = 0;

  // camera
  Camera camera_{glm::vec3(0.0f, 1.0f, 6.0f)};

  float deltaTime_ = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;
};

int main() try {
  App app(1920, 1080, "Grass Renderer");
  app.run();
} catch (const std::exception& e) {
  fmt::print(stderr, "Error: {}\n", e.what());
} catch (...) {
  fmt::print(stderr, "Unknown exception!\n");
}
// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
void processInput(GLFWwindow* window)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  auto& camera = app_ptr->camera();
  const auto deltaTime = app_ptr->deltaTime();

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.move(Camera::Movement::forward, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.move(Camera::Movement::backward, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.move(Camera::Movement::left, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.move(Camera::Movement::right, deltaTime);
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  auto& camera = app_ptr->camera();
  const auto width = app_ptr->width();
  const auto height = app_ptr->height();

  static bool firstMouse = true;
  static float lastX = width / 2.0f;
  static float lastY = height / 2.0f;

  if (firstMouse) {
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    firstMouse = false;
  }

  auto xoffset = static_cast<float>(xpos - lastX);
  auto yoffset = static_cast<float>(
      lastY - ypos); // reversed since y-coordinates go from bottom to top

  lastX = static_cast<float>(xpos);
  lastY = static_cast<float>(ypos);

  camera.mouse_movement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  auto& camera = app_ptr->camera();

  camera.mouse_scroll(static_cast<float>(yoffset));
}
