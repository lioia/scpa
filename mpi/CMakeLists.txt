find_package(MPI REQUIRED)

add_executable(scpa-mpi main.c ../common/utils.c ../common/matrix.c utils.c)

target_link_libraries(scpa-mpi PUBLIC MPI::MPI_C)

if(NOT TARGET OpenMP::OpenMP_CXX)
  find_package(Threads REQUIRED)
  add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
  set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS
                                                  ${OpenMP_CXX_FLAGS})
  set_property(
    TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_LINK_LIBRARIES
                                       ${OpenMP_CXX_FLAGS} Threads::Threads)

endif()

add_executable(scpa-mpi-omp main.c ../common/utils.c ../common/matrix.c utils.c)

target_link_libraries(scpa-mpi-omp PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(scpa-mpi-omp PUBLIC MPI::MPI_C)
