find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(scpa-mpi main.c ../common/utils.c ../common/matrix.c)
add_executable(scpa-mpi-omp main.c ../common/utils.c ../common/matrix.c)

target_link_libraries(scpa-mpi PUBLIC MPI::MPI_C)
target_link_libraries(scpa-mpi-omp PUBLIC MPI::MPI_C)

if(OpenMP_C_FOUND)
  target_link_libraries(scpa-mpi-omp PUBLIC OpenMP::OpenMP_C)
endif()
