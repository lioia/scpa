cmake_minimum_required(VERSION 3.7)
project(scpa)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  add_compile_definitions(DEBUG)
endif()

set(CMAKE_CXX_FLAGS "-O3")

add_subdirectory(mpi)
add_subdirectory(cuda)
