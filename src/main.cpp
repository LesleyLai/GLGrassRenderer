#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <random>
#include <string_view>
#include <vector>

#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"

#include <fmt/format.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow* window);

// Indirect drawing structure
struct NumBlades {
  std::uint32_t vertexCount = 5;
  std::uint32_t instanceCount = 1;
  std::uint32_t firstVertex = 0;
  std::uint32_t firstInstance = 0;
};

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

[[nodiscard]] std::unique_ptr<Mesh> generate_terrain_model()
{
  constexpr std::size_t terrian_x_max = 20;
  constexpr std::size_t terrian_y_max = 20;

  std::vector<Vertex> verts;
  std::vector<std::uint32_t> indices;
  verts.reserve(terrian_x_max * terrian_y_max * 4);
  indices.reserve(terrian_x_max * terrian_y_max * 6);

  for (std::size_t x = 0; x < terrian_x_max; ++x) {
    const float x_offset = -static_cast<float>(terrian_x_max) + 2 * x;
    for (std::size_t y = 0; y < terrian_y_max; ++y) {
      const float y_offset = -static_cast<float>(terrian_y_max) + 2 * y;
      verts.push_back({{1.0f + x_offset, 0.0f, 1.0f + y_offset}, {1.0f, 0.0f}});
      verts.push_back(
          {{1.0f + x_offset, 0.0f, -1.0f + y_offset}, {1.0f, 1.0f}});
      verts.push_back(
          {{-1.0f + x_offset, 0.0f, -1.0f + y_offset}, {0.0f, 1.0f}});
      verts.push_back(
          {{-1.0f + x_offset, 0.0f, 1.0f + y_offset}, {0.0f, 0.0f}});

      const auto index_base =
          static_cast<std::uint32_t>(4 * (y + x * terrian_y_max));
      indices.push_back(index_base);
      indices.push_back(index_base + 1);
      indices.push_back(index_base + 3);
      indices.push_back(index_base + 1);
      indices.push_back(index_base + 2);
      indices.push_back(index_base + 3);
    }
  }

  return std::make_unique<Mesh>(std::move(verts), std::move(indices),
                                "GrassGreenTexture0001.jpg");
}

class App {
public:
  using DeltaDuration = std::chrono::duration<float, std::milli>;

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
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetScrollCallback(window_, scroll_callback);

    glfwSetWindowUserPointer(window_, this);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
      fmt::print(stderr, "Failed to initialize GLAD\n");
      glfwTerminate();
      std::exit(1);
    }

    // Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Load Fonts
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 30.0f);

    glEnable(GL_DEPTH_TEST);
    {
      terrain_model_ = generate_terrain_model();
      terrain_shader_ = ShaderBuilder{}
                            .load("land.vert.glsl", Shader::Type::Vertex)
                            .load("land.frag.glsl", Shader::Type::Fragment)
                            .build();
      terrain_shader_.use();
      terrain_shader_.setInt("texture1", 0);
    }

    {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float> orientation_dis(0,
                                                            glm::pi<float>());
      std::uniform_real_distribution<float> height_dis(0.6f, 1.2f);
      std::uniform_real_distribution<float> dis(-1, 1);

      for (int i = -200; i < 200; ++i) {
        for (int j = -200; j < 200; ++j) {
          const auto x = static_cast<float>(i) / 10 - 1 + dis(gen) * 0.1f;
          const auto y = static_cast<float>(j) / 10 - 1 + dis(gen) * 0.1f;
          const auto blade_height = height_dis(gen);

          blades_.emplace_back(
              glm::vec4(x, 0, y, orientation_dis(gen)),
              glm::vec4(x, blade_height, y, blade_height),
              glm::vec4(x, blade_height, y, 0.1),
              glm::vec4(0, blade_height, 0, 0.7 + dis(gen) * 0.3));
        }
      }

      glPatchParameteri(GL_PATCH_VERTICES, 1);

      glGenVertexArrays(1, &grass_vao_);
      glBindVertexArray(grass_vao_);

      unsigned int grass_input_buffer;
      glGenBuffers(1, &grass_input_buffer);

      glBindBuffer(GL_SHADER_STORAGE_BUFFER, grass_input_buffer);
      glBufferData(GL_SHADER_STORAGE_BUFFER,
                   static_cast<GLsizei>(blades_.size() * sizeof(Blade)),
                   blades_.data(), GL_DYNAMIC_COPY);

      unsigned int grass_output_buffer;
      glGenBuffers(1, &grass_output_buffer);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, grass_output_buffer);
      glBufferData(GL_SHADER_STORAGE_BUFFER,
                   static_cast<GLsizei>(blades_.size() * sizeof(Blade)),
                   nullptr, GL_STREAM_DRAW);

      NumBlades numBlades;
      unsigned int grass_indirect_buffer;
      glGenBuffers(1, &grass_indirect_buffer);
      glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grass_indirect_buffer);
      glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(NumBlades), &numBlades,
                   GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, grass_output_buffer);

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
          ShaderBuilder{}
              .load("grass.comp.glsl", Shader::Type::Compute)
              .build();
      grass_compute_shader_.use();
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grass_input_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grass_output_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grass_indirect_buffer);

      grass_shader_ = ShaderBuilder{}
                          .load("grass.vert.glsl", Shader::Type::Vertex)
                          .load("grass.tesc.glsl", Shader::Type::TessControl)
                          .load("grass.tese.glsl", Shader::Type::TessEval)
                          .load("grass.frag.glsl", Shader::Type::Fragment)
                          .build();
    }

    glGenBuffers(1, &cameraUniformBuffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBuffer_);
    glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformBuffer_);
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void run()
  {
    last_frame_ = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window_)) {
      const auto current_time = std::chrono::high_resolution_clock::now();
      delta_time_ = current_time - last_frame_;
      last_frame_ = current_time;

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        ImGui::Begin("Control");

        ImGui::Text("%.3f ms/frame (%.1f FPS)", delta_time_.count(),
                    1000.f / delta_time_.count());

        static float camera_speed = camera_.speed();
        ImGui::SliderFloat("Camera Speed", &camera_speed, 0.5, 30, "%.4f",
                           2.0f);
        camera_.set_speed(camera_speed);

        ImGui::End();
      }

      ImGui::Render();

      process_input(window_);

      // render
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // activate shader
      terrain_shader_.use();

      // camera/view transformation
      glm::mat4 view = camera_.view_matrix();
      glm::mat4 projection = glm::perspective(glm::radians(camera_.zoom()),
                                              static_cast<float>(width_) /
                                                  static_cast<float>(height_),
                                              0.1f, 100.0f);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUniformBuffer_);
      glBindBuffer(GL_UNIFORM_BUFFER, cameraUniformBuffer_);

      glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &view);
      glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, &projection);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      // render quad
      terrain_model_->render();

      glBindVertexArray(grass_vao_);

      { // launch compute shaders!
        grass_compute_shader_.use();
        grass_compute_shader_.setFloat("current_time", glfwGetTime());

        grass_compute_shader_.setFloat("delta_time", delta_time_.count() / 1e3);

        glDispatchCompute(static_cast<GLuint>(blades_.size()), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
      }

      grass_shader_.use();
      glDrawArraysIndirect(GL_PATCHES, reinterpret_cast<void*>(0));

      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window_);
      glfwPollEvents();
    }
  }

  ~App()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

  [[nodiscard]] DeltaDuration delta_time() const noexcept
  {
    return delta_time_;
  }

  [[nodiscard]] int width() const noexcept
  {
    return width_;
  }

  [[nodiscard]] int height() const noexcept
  {
    return height_;
  }

  [[nodiscard]] bool right_clicking() const
  {
    return right_clicking_;
  }

  void set_right_clicking(bool right_clicking)
  {
    right_clicking_ = right_clicking;
  }

private:
  GLFWwindow* window_;
  int width_;
  int height_;

  bool right_clicking_ = false;

  std::unique_ptr<Mesh> terrain_model_;
  ShaderProgram terrain_shader_{};

  unsigned int grass_vao_ = 0;
  ShaderProgram grass_shader_{};
  ShaderProgram grass_compute_shader_{};
  std::vector<Blade> blades_;

  unsigned int cameraUniformBuffer_ = 0;

  // camera
  Camera camera_{glm::vec3(0.0f, 1.0f, 6.0f)};

  DeltaDuration delta_time_;
  std::chrono::high_resolution_clock::time_point last_frame_;
};

int main() try {
  App app(1920, 1080, "Grass Renderer");
  app.run();
} catch (const std::exception& e) {
  fmt::print(stderr, "Error: {}\n", e.what());
} catch (...) {
  fmt::print(stderr, "Unknown exception!\n");
}

void process_input(GLFWwindow* window)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  auto& camera = app_ptr->camera();
  const auto delta_time = app_ptr->delta_time();

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.move(Camera::Movement::forward, delta_time);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.move(Camera::Movement::backward, delta_time);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.move(Camera::Movement::left, delta_time);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.move(Camera::Movement::right, delta_time);
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
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
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

  // reversed since y-coordinates go from bottom to top
  const auto xoffset = static_cast<float>(xpos) - lastX;
  const auto yoffset = lastY - static_cast<float>(ypos);

  lastX = static_cast<float>(xpos);
  lastY = static_cast<float>(ypos);

  if (app_ptr->right_clicking()) {
    camera.mouse_movement(xoffset, yoffset);
  }
}

void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int /*mods*/)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    switch (action) {
    case GLFW_PRESS:
      app_ptr->set_right_clicking(true);
      break;
    case GLFW_RELEASE:
      app_ptr->set_right_clicking(false);
    }
  }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
  auto* app_ptr = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  auto& camera = app_ptr->camera();

  camera.mouse_scroll(static_cast<float>(yoffset));
}
