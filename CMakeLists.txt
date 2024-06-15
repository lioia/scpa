cmake_minimum_required(VERSION 3.15)
project(scpa LANGUAGES C)

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # In debug builds, disable optimization, include debug information and add new definition
  add_compile_definitions(DEBUG)
  add_compile_options(-g)
  add_compile_options(-O0)
else()
  # otherwise, enable optimization and architecture specific instructions
  add_compile_options(-O3)
  add_compile_options(-march=native)
endif()

# Create compile_commands.json, used by clangd LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add MPI executable
add_executable(scpa-mpi src/mpi/main-collective.c 
  src/common/utils.c src/common/matrix.c src/mpi/utils.c)

# Link math and MPI to MPI executable
target_link_libraries(scpa-mpi PUBLIC MPI::MPI_C m)

# Add OpenMP executable
add_executable(scpa-omp src/omp/main.c 
  src/common/utils.c src/common/matrix.c)

# Link math to OpenMP executable (required)
target_link_libraries(scpa-omp PUBLIC m)

if(OpenMP_C_FOUND)
  # Link OpenMP to OpenMP executable (if available)
  target_link_libraries(scpa-omp PUBLIC OpenMP::OpenMP_C)

  # Add MPI-OpenMP executable
  add_executable(scpa-mpi-omp src/mpi/main-collective.c 
    src/common/utils.c src/common/matrix.c src/mpi/utils.c)

  # Link math, MPI and OpenMP to MPI-OpenMP executable
  target_link_libraries(scpa-mpi-omp PUBLIC MPI::MPI_C OpenMP::OpenMP_C m)
endif()
