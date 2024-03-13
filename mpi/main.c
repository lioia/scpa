#include <mpi.h>
#include <mpi_proto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/utils.h"

int main(int argc, char **argv) {
  // Variable Declaration
  // int m = 0, n = 0, k = 0; // Matrix dimension
  // unsigned long seed = 0;  // Seed for random generation
  // float *a, *b, *c;        // Matrices
  // gen_type_t type = ZERO;  // Generation type
  // int rank, p;             // Process rank and number of processes
  // MPI_Comm comm;           // Custom communicator
  // // TODO:different variants (2D on A, 2D on B 2D on C)
  // if (argc != 6) {
  //   puts("Usage: mpirun -n <p> ./build/mpi/scpa-mpi <m> <n> <k> <initial-seed> <type>");
  //   exit(EXIT_FAILURE);
  // }
  // // Variable Initialization
  // // // TODO: errno check for ERANGE
  // m = strtol(argv[1], NULL, 10);
  // n = strtol(argv[2], NULL, 10);
  // k = strtol(argv[3], NULL, 10);
  // seed = strtol(argv[4], NULL, 10);
  // if (!strncmp(argv[5], "zero", 4))
  //   type = ZERO;
  // else if (!strncmp(argv[5], "index", 5))
  //   type = INDEX;
  // else if (!strncmp(argv[5], "random", 6))
  //   type = RANDOM;
  // else
  //   printf("Unrecognized type (%s), using ZERO as default\n", argv[5]);
  // MPI_Init(&argc, &argv);
  // MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  // MPI_Comm_rank(comm, &rank);
  // MPI_Comm_size(comm, &p);
  // srand(seed + p);             // Seed is the initial seed + current process
  // int cols_block_size = n / p; // Number of cols for each process
  // int rows_block_size = m / p; // Number of rows for each process
  // // Rest of the division: used to fairly divide the work between all processes
  // int rest_cols = n % p;
  // int rest_rows = m % p;
  // // Start indices
  // int start_cols = rank * cols_block_size;
  // int start_rows = rank * rows_block_size;
  // // End indices
  // int end_cols = 0;
  // int end_rows = 0;
  // if (rank < rest_cols) {
  //   start_cols += rank;
  //   end_cols += 1;
  // } else {
  //   start_cols += rest_cols;
  // }
  // if (rank < rest_rows) {
  //   start_rows += rank;
  //   end_rows += 1;
  // } else {
  //   start_rows += rest_rows;
  // }
  // // End indices
  // end_cols += start_cols + cols_block_size;
  // end_rows += start_rows + rows_block_size;
  // printf("Process %d: rows (%d %d); cols (%d %d)\n", rank, start_rows, end_rows, start_cols, end_cols);
  // MPI_Finalize();
  return 0;
}
