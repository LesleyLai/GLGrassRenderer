include("CPM")

#configure directories
set(THIRD_PARTY_MODULE_PATH "${PROJECT_SOURCE_DIR}/third-party")

CPMAddPackage(
        NAME fmt
        VERSION 11.1.4
        GIT_TAG 11.1.4
        GITHUB_REPOSITORY fmtlib/fmt
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.4
        OPTIONS
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_DOCS OFF"
        "GLFW_INSTALL OFF"
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

CPMAddPackage(
        NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 1.0.1
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

# Glad
add_library(glad
        "${THIRD_PARTY_MODULE_PATH}/glad/src/glad.c")

target_include_directories(glad
        PUBLIC
        "${THIRD_PARTY_MODULE_PATH}/glad/include")

CPMAddPackage(
        NAME stb
        GITHUB_REPOSITORY nothings/stb
        GIT_TAG 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
)

# Imgui
add_library(imgui
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui.cpp"
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_demo.cpp"
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_draw.cpp"
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_widgets.cpp"
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_impl_glfw.cpp"
        "${THIRD_PARTY_MODULE_PATH}/imgui/imgui_impl_opengl3.cpp")

target_link_libraries(imgui
        glfw glad)

target_compile_definitions(imgui PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_include_directories(imgui
        SYSTEM PUBLIC
        "${THIRD_PARTY_MODULE_PATH}/imgui")

# STB
add_library(stb
        INTERFACE
)

target_include_directories(stb
        SYSTEM INTERFACE
        "${THIRD_PARTY_MODULE_PATH}/stb")
