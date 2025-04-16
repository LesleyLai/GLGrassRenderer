#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <string_view>
#include <vector>
#include <memory>

#include "camera.hpp"
#include "grasses.hpp"
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

// clang-format off
constexpr float skybox_vertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
// clang-format on

void load_gl()
{
  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    fmt::print(stderr, "Failed to initialize GLAD\n");
    glfwTerminate();
    std::exit(1);
  }
}

[[nodiscard]] std::unique_ptr<Mesh> generate_terrain_model()
{
  constexpr std::size_t terrian_x_max = 20;
  constexpr std::size_t terrian_y_max = 20;

  std::vector<Vertex> verts;
  std::vector<std::uint32_t> indices;
  verts.reserve(terrian_x_max * terrian_y_max * 4);
  indices.reserve(terrian_x_max * terrian_y_max * 6);

  for (std::size_t x = 0; x < terrian_x_max; ++x) {
    const float x_offset =
        -static_cast<float>(terrian_x_max) + 2 * static_cast<float>(x);
    for (std::size_t y = 0; y < terrian_y_max; ++y) {
      const float y_offset =
          -static_cast<float>(terrian_y_max) + 2 * static_cast<float>(y);
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

unsigned int load_cubemap(std::vector<std::string> faces)
{
  unsigned int texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char* data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap texture failed to load at path: " << faces[i]
                << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return texture_id;
}

void init_imgui(GLFWwindow* window)
{
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
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");

  // Load Fonts
  io.Fonts->AddFontFromFileTTF("./fonts/Roboto-Medium.ttf", 30.0f);
}

void destroy_imgui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

class App {
public:
  using DeltaDuration = std::chrono::duration<double, std::milli>;

  App(int width, int height, std::string_view title)
      : width_{width}, height_{height}, delta_time_{}
  {
    init_window(title);
    load_gl();

    init_imgui(window_);

    glEnable(GL_DEPTH_TEST);

    init_skybox();
    init_terrain();
    grasses_.init();
    init_camera_uniform_buffer();
  }

  void init_window(const std::string_view& title)
  {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width_, height_, title.data(), nullptr, nullptr);
    if (window_ == nullptr) {
      fmt::print(stderr, "Failed to create GLFW window\n");
      glfwTerminate();
      exit(1);
    }
    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetScrollCallback(window_, scroll_callback);

    glfwSetWindowUserPointer(window_, this);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void init_skybox()
  {
    skybox_texture_ = load_cubemap(
        {"textures/ely_hills/hills_rt.tga", "textures/ely_hills/hills_lf.tga",
         "textures/ely_hills/hills_up.tga", "textures/ely_hills/hills_dn.tga",
         "textures/ely_hills/hills_ft.tga", "textures/ely_hills/hills_bk.tga"});

    glGenVertexArrays(1, &skybox_vao_);
    glBindVertexArray(skybox_vao_);

    glDisable(GL_CULL_FACE);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(skybox_vertices)),
                 skybox_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          reinterpret_cast<void*>(0));

    skybox_shader_ = ShaderBuilder{}
                         .load("skybox.vert.glsl", Shader::Type::Vertex)
                         .load("skybox.frag.glsl", Shader::Type::Fragment)
                         .build();

    skybox_shader_.use();
    skybox_shader_.setInt("skybox", 0);
  }

  void init_terrain()
  {
    terrain_model_ = generate_terrain_model();
    terrain_shader_ = ShaderBuilder{}
                          .load("land.vert.glsl", Shader::Type::Vertex)
                          .load("land.frag.glsl", Shader::Type::Fragment)
                          .build();
    terrain_shader_.use();
    terrain_shader_.setInt("texture1", 0);
  }

  void init_camera_uniform_buffer()
  {
    glGenBuffers(1, &camera_uniform_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_uniform_buffer_);
    /*
      mat4 view; // 64
      mat4 proj; // 64
      vec3 position; // 16
    */
    glBufferData(GL_UNIFORM_BUFFER, 140, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_uniform_buffer_);
  }

  void run()
  {
    last_frame_ = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window_)) {
      const auto current_time = std::chrono::steady_clock::now();
      delta_time_ = current_time - last_frame_;
      last_frame_ = current_time;

      process_input(window_);
      render();

      glfwSwapBuffers(window_);
      glfwPollEvents();
    }
  }

  void render()
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_gui();
    render_scene();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void render_scene()
  {
    // camera/view transformation
    glm::vec3 position = camera_.position();
    glm::mat4 view = camera_.view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera_.zoom()),
        static_cast<float>(width_) / static_cast<float>(height_), 0.1f, 100.0f);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_uniform_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_uniform_buffer_);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &view);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, &projection);
    glBufferSubData(GL_UNIFORM_BUFFER, 128, 12, &position);

    grasses_.update(delta_time_);

    // Skybox
    glDepthMask(GL_FALSE);
    skybox_shader_.use();
    glBindVertexArray(skybox_vao_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    // Terrain
    terrain_shader_.use();
    terrain_model_->render();

    grasses_.render();
  }

  void draw_gui()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", delta_time_.count(),
                1000.f / delta_time_.count());

    static float camera_speed = camera_.speed();
    ImGui::SliderFloat("Camera Speed", &camera_speed, 0.5, 30, "%.4f", 2.0f);
    camera_.set_speed(camera_speed);

    if (ImGui::CollapsingHeader("Wind")) {
      ImGui::SliderFloat("Magnitude", &grasses_.wind_magnitude, 0.5f, 3,
                         "%.4f");
      ImGui::SliderFloat("Wave Length", &grasses_.wind_wave_length, 0.5f, 2,
                         "%.4f");
      ImGui::SliderFloat("Wave Period", &grasses_.wind_wave_period, 0.5f, 2,
                         "%.4f");
    }

    ImGui::End();

    ImGui::Render();
  }

  ~App()
  {
    destroy_imgui();
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

  [[nodiscard]] bool right_clicking() const noexcept
  {
    return right_clicking_;
  }

  void set_right_clicking(bool right_clicking) noexcept
  {
    right_clicking_ = right_clicking;
  }

private:
  GLFWwindow* window_ = nullptr;
  int width_ = 0;
  int height_ = 0;

  bool right_clicking_ = false;

  std::unique_ptr<Mesh> terrain_model_;
  ShaderProgram terrain_shader_{};

  Grasses grasses_;

  ShaderProgram skybox_shader_{};
  unsigned int skybox_vao_ = 0;

  unsigned int camera_uniform_buffer_ = 0;

  // camera
  Camera camera_{glm::vec3(0.0f, 1.0f, 6.0f)};

  DeltaDuration delta_time_;
  std::chrono::steady_clock::time_point last_frame_;

  unsigned int skybox_texture_ = 0;
};

int main()
try {
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
  const auto f_width = static_cast<float>(app_ptr->width());
  const auto f_height = static_cast<float>(app_ptr->height());

  static bool firstMouse = true;
  static float lastX = f_width / 2.0f;
  static float lastY = f_height / 2.0f;

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
