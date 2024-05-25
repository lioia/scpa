cmake_minimum_required(VERSION 3.15)
project(scpa LANGUAGES C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
endif()

# Create compile_commands.json, used by clangd LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-O3)
add_compile_options(-march=native)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src/generator)
add_subdirectory(src/mpi)
add_subdirectory(src/omp)
