#configure directories
set (THIRD_PARTY_MODULE_PATH "${PROJECT_SOURCE_DIR}/third-party")

# Glad
add_library (glad
    "${THIRD_PARTY_MODULE_PATH}/glad/src/glad.c")

target_include_directories (glad
    PUBLIC
    "${THIRD_PARTY_MODULE_PATH}/glad/include")

# Imgui
add_library (imgui
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui.cpp"
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_demo.cpp"
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_draw.cpp"
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_widgets.cpp"
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_impl_glfw.cpp"
    "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_impl_opengl3.cpp")

target_link_libraries(imgui
    CONAN_PKG::glfw glad)

target_compile_definitions(imgui PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_include_directories (imgui
    PUBLIC
    "${THIRD_PARTY_MODULE_PATH}/imgui")
