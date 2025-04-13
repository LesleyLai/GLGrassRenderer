add_executable(app
        "camera.hpp"
        "camera.cpp"
        "main.cpp"
        "model.hpp"
        "model.cpp"
        "shader.hpp"
        "shader.cpp"
        "texture.hpp"
        "texture.cpp"
        grasses.cpp grasses.hpp)
target_link_libraries(app
        PRIVATE compiler_warnings
        glm::glm glfw fmt::fmt stb glad imgui
        )
add_dependencies(app assets)
add_clangformat(app)
