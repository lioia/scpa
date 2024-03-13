find_package(MPI REQUIRED)

add_executable(scpa-mpi main.c)

target_link_libraries(scpa-mpi PUBLIC MPI::MPI_C)
