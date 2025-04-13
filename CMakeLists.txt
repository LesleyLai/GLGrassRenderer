cmake_minimum_required(VERSION 3.8)
project(GLGrassRenderer VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(BP_ENABLE_CLANG_TIDY "Enable testing with clang-tidy" OFF)
option(BP_ENABLE_CPPCHECK "Enable testing with cppcheck" OFF)

include("compiler")
include("clangformat")

if (BP_ENABLE_CLANG_TIDY)
include("ClangTidy")
endif()

if (BP_ENABLE_CPPCHECK)
include("cppcheck")
endif()

add_custom_target(assets
       COMMAND ${CMAKE_COMMAND} -E copy_directory
       ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/bin
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_subdirectory(third-party)
add_subdirectory(src)
