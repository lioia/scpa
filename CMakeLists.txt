cmake_minimum_required(VERSION 3.15)
project(scpa LANGUAGES C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
endif()

# Create compile_commands.json, used by clangd LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "-O3")

add_subdirectory(mpi)
add_subdirectory(omp)

add_executable(scpa-matrix-generator common/matrix_generator.c common/matrix.c
                                     common/utils.c)
