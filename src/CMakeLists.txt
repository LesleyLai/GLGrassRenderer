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
    )
target_link_libraries(app
    PRIVATE compiler_warnings
    CONAN_PKG::glm CONAN_PKG::glfw CONAN_PKG::fmt CONAN_PKG::stb glad imgui
    )
add_dependencies(app assets)
add_clangformat(app)
