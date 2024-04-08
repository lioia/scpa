cmake_minimum_required(VERSION 3.15)
project(scpa LANGUAGES C)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  add_compile_definitions(DEBUG)
endif()

set(CMAKE_C_FLAGS "-O3")

add_subdirectory(mpi)
add_subdirectory(omp)

add_executable(scpa-matrix-generator common/matrix_generator.c common/matrix.c
                                     common/utils.c)
