#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../common/matrix.h"
#include "../common/utils.h"
#include "utils.h"

// NOTE: Block Distribution on C
// root process generates the entire matrices and distribute the data
// every process allocates the entire C matrix

int main(int argc, char **argv) {
  // Variable Declaration
  int m, n, k;                                    // Matrix dimension (from arguments)
  int p, t, rank;                                 // Number of processes, threads and this process rank
  float *a, *b, *c, *c_serial;                    // Matrices
  float *local_a, *local_b, *local_c;             // Local Matrices
  MPI_Comm topology_comm, temp_comm;              // Custom communicator
  int dims[2], periods[2], coords[2];             // Topology dimensions and coordinates
  int start_rows, end_rows, start_cols, end_cols; // Block distribution
  int n_rows, n_cols;                             // Local matrix dimension
  MPI_Datatype row_type, row_type_resized;        // Row Datatype
  MPI_Datatype col_type, col_type_resized;        // Column Datatype
  int *a_counts, *a_displs, *b_counts, *b_displs; // Scatterv parameters
  void *sendbuf, *recvbuf;                        // Scatterv and Reduce parameter
  double start_time;                              // Start timer
  float error;                                    // Difference between serial and parallel computation
  enum gen_type_t gen_type;                       // Generation type
  stats_t stats;                                  // Stats

// Based on the version being used, a different number of arguments is required
// In the OpenMP version the number of threads must be specified as an argument
#ifdef _OPENMP
  if (argc != 5) {
    puts("Usage: mpirun -n <p> ./build/mpi/scpa-mpi-omp <m> <n> <k> <t>");
#else
  if (argc != 4) {
    puts("Usage: mpirun -n <p> ./build/mpi/scpa-mpi <m> <n> <k>");
#endif
    exit(EXIT_FAILURE);
  }
  // Variable Initialization

  // Parsing arguments
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  memset(&stats, 0, sizeof(stats));

#ifdef _OPENMP
  t = parse_int_arg(argv[4]);
  // OpenMP initialization
  omp_set_num_threads(t);
#else
  t = 0;
#endif
  stats.threads = t;

  // MPI Initialization
  MPI_Init(&argc, &argv);
  start_time = MPI_Wtime();
  MPI_Comm_dup(MPI_COMM_WORLD, &temp_comm);
  // Getting basics information; using a temp communicator even though a new comm will be created by the topology
  MPI_Comm_rank(temp_comm, &rank);
  MPI_Comm_size(temp_comm, &p);
  stats.processes = p;

#ifdef _OPENMP
  if (rank == 0)
    printf("Running with OpenMP\n");
#endif

  // 2D Cartesian Topology
  dims[0] = dims[1] = 0;                                           // Default assignment
  periods[0] = periods[1] = 0;                                     // No wrapping
  MPI_Dims_create(p, 2, dims);                                     // Create dimensions
  MPI_Cart_create(temp_comm, 2, dims, periods, 0, &topology_comm); // Create topology
  MPI_Cart_coords(topology_comm, rank, 2, coords);                 // Get coords in topology

  // Calculate parameters for block distribution of this process
  calculate_start_end(m, dims[0], coords[0], &start_rows, &end_rows);
  calculate_start_end(n, dims[1], coords[1], &start_cols, &end_cols);
  n_rows = end_rows - start_rows; // Matrix A rows to receive
  n_cols = end_cols - start_cols; // Matrix B cols to receive

  // Create new DataType for row
  MPI_Type_vector(1, k, k, MPI_FLOAT, &row_type);
  MPI_Type_create_resized(row_type, 0, k * sizeof(*a), &row_type_resized);
  MPI_Type_commit(&row_type_resized);

  // Create new DataType for column
  MPI_Type_vector(k, 1, n, MPI_FLOAT, &col_type);
  MPI_Type_create_resized(col_type, 0, 1 * sizeof(*b), &col_type_resized);
  MPI_Type_commit(&col_type_resized);

#ifndef DEBUG
  gen_type = RANDOM;
#else
  gen_type = INDEX;
#endif
  // Matrix allocation
  if (rank == 0) {
    // Rank 0 creates the matrices
    a = matrix_init(m, k, gen_type, SEED + 0);
    b = matrix_init(k, n, gen_type, SEED + 1);
    c_serial = matrix_init(m, n, ZERO, 0);
    if (a == NULL || b == NULL || c_serial == NULL) {
      perror("Error creating matrices");
      MPI_Abort(topology_comm, EXIT_FAILURE);
    }
  }

  // Every process allocates the local matrices
  local_a = matrix_init(n_rows, k, ZERO, 0);
  local_b = matrix_init(k, n_cols, ZERO, 0);
  local_c = matrix_init(m, n, ZERO, 0);
  c = matrix_init(m, n, ZERO, 0);
  if (local_a == NULL || local_b == NULL || local_c == NULL || c == NULL) {
    perror("Error allocating local matrices");
    MPI_Abort(topology_comm, EXIT_FAILURE);
  }

  MPI_Barrier(topology_comm);
  stats.generation_time = MPI_Wtime() - start_time; // Generation time

  start_time = MPI_Wtime();

  // Scatterv parameters
  a_counts = malloc(sizeof(*a_counts) * p);
  a_displs = malloc(sizeof(*a_displs) * p);
  b_counts = malloc(sizeof(*b_counts) * p);
  b_displs = malloc(sizeof(*b_displs) * p);
  if (a_counts == NULL || a_displs == NULL || b_counts == NULL || b_displs == NULL) {
    perror("Error allocating parameters for Scatterv");
    MPI_Abort(topology_comm, EXIT_FAILURE);
  }

  for (int i = 0; i < p; i++) {
    // Calculating block distribution of every process
    int i_coords[2];              // Coordinate for process i
    int i_start_rows, i_end_rows; // Row distribution for process i
    int i_start_cols, i_end_cols; // Column distribution for process i
    MPI_Cart_coords(topology_comm, i, 2, i_coords);
    calculate_start_end(m, dims[0], i_coords[0], &i_start_rows, &i_end_rows);
    calculate_start_end(n, dims[1], i_coords[1], &i_start_cols, &i_end_cols);
    a_counts[i] = i_end_rows - i_start_rows; // Rows to send
    a_displs[i] = i_start_rows;              // Row offset
    b_counts[i] = i_end_cols - i_start_cols; // Columns to send
    b_displs[i] = i_start_cols;              // Column offset
  }

  // Scatter Matrix A and B
  sendbuf = rank == 0 ? a : NULL;
  MPI_Scatterv(sendbuf, a_counts, a_displs, row_type_resized, local_a, n_rows * k, MPI_FLOAT, 0, topology_comm);
  sendbuf = rank == 0 ? b : NULL;
  MPI_Scatterv(sendbuf, b_counts, b_displs, col_type_resized, local_b, n_cols * k, MPI_FLOAT, 0, topology_comm);

  MPI_Barrier(topology_comm);
  stats.first_communication_time = MPI_Wtime() - start_time; // Matrices communication time

  start_time = MPI_Wtime();

  // If there is only one process, Scatterv becomes a no-operation
  // Matrix B has to be manually transposed
  if (p == 0)
    matrix_transpose(b, local_b, k, n_cols);

  // Parallel computation

  matrix_parallel_mult(local_a, local_b, local_c, n_rows, n_cols, k, n, start_rows, start_cols);

  MPI_Barrier(topology_comm);                     // Every process has terminated the execution of the parallel code
  stats.parallel_time = MPI_Wtime() - start_time; // Parallel computation

  start_time = MPI_Wtime();

  // Reduce to root process 0
  recvbuf = rank == 0 ? c : NULL;
  MPI_Allreduce(local_c, c, m * n, MPI_FLOAT, MPI_SUM, topology_comm);

  MPI_Barrier(topology_comm);
  stats.second_communication_time = MPI_Wtime() - start_time; // Final matrix communication time

  // Cleanup MPI environment
  MPI_Type_free(&row_type_resized);
  MPI_Type_free(&col_type_resized);
  MPI_Comm_free(&topology_comm);
  MPI_Comm_free(&temp_comm);
  MPI_Finalize();

  // Program has effectively finished
  // What follows is additional tasks for the serial check and stats writing
  // These tasks will be done only by root process
  if (rank != 0)
    goto close;

  if (root_tasks(a, b, c, c_serial, m, n, k, &stats, MPIv1) != 0)
    return EXIT_FAILURE;

close:
  // Cleanup
  if (rank == 0) {
    free(a);
    free(b);
    free(c_serial);
  }
  free(local_a);
  free(local_b);
  free(local_c);
  free(c);
  free(a_counts);
  free(a_displs);
  free(b_counts);
  free(b_displs);
  return 0;
}
