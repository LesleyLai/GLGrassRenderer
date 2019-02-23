#configure directories
set (THIRD_PARTY_MODULE_PATH "${PROJECT_SOURCE_DIR}/third-party")

# Glad
add_library (glad
    "${THIRD_PARTY_MODULE_PATH}/glad/src/glad.c")

target_include_directories (glad
    PUBLIC
    "${THIRD_PARTY_MODULE_PATH}/glad/include")
