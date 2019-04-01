# Compiler specific settings

if(compiler_included)
  return()
endif()
set(compiler_included true)

# Link this 'library' to use the standard warnings
add_library(compiler_warnings INTERFACE)

if(MSVC)
  target_compile_options(compiler_warnings INTERFACE /W3 "/permissive-")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compiler_warnings
                         INTERFACE -Wall
                                   -Wextra
                                   -Wshadow
                                   -Wnon-virtual-dtor
                                   -Wold-style-cast
                                   -Wcast-align
                                   -Wunused
                                   -Woverloaded-virtual
                                   -Wpedantic
                                   -Wconversion
                                   -Wsign-conversion
                                   -Wnull-dereference
                                   -Wdouble-promotion
                                   -Wformat=2)

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(compiler_warnings
                           INTERFACE -Wmisleading-indentation
                                     -Wduplicated-cond
                                     -Wduplicated-branches
                                     -Wlogical-op
                                     -Wuseless-cast
                           )
  endif()
endif()
