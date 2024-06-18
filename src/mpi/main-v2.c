#include <mpi.h>
#include <mpi_proto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../common/matrix.h"
#include "../common/utils.h"
#include "utils.h"

// NOTE: Block Distribution on C with focus on memory usage
// - root process generates seeds and distributes to the other processes
// - every process generates its data by allocating only the memory required
// - results are then collected by the root process

int main(int argc, char **argv) {
  // Variable Declaration
  int m, n, k, iteration;                         // Matrix dimension (from arguments) and number of iteration
  int p, t, rank;                                 // Number of processes, threads and this process rank
  float *local_a, *local_b, *local_b_t, *c;       // Local Matrices
  float *a, *b, *c_serial;                        // Global matrices (used by root for gather and serial check)
  int *seeds_per_dim, *seeds, local_seeds[2];     // Seeds to distributes and local seeds received
  MPI_Comm topology_comm, temp_comm;              // Custom communicator
  int dims[2], periods[2], coords[2];             // Topology dimensions and coordinates
  int start_rows, end_rows, start_cols, end_cols; // Block distribution
  int n_rows, n_cols;                             // Local matrix dimension
  void *sendbuf;                                  // Scatter parameter
  int offset;                                     // Offset used to generate global matrices
  double start_time;                              // Start timer
  enum gen_type_t gen_type;                       // Generation type
  stats_t stats;                                  // Stats

// Based on the version being used, a different number of arguments is required
// In the OpenMP version the number of threads must be specified as an argument
#ifdef _OPENMP
  if (argc != 6) {
    puts("Usage: mpirun -n <p> ./build/mpi/scpa-mpi-omp-v2 <m> <n> <k> <t> <iteration>");
#else
  if (argc != 5) {
    puts("Usage: mpirun -n <p> ./build/mpi/scpa-mpi-v2 <m> <n> <k> <iteration>");
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
  iteration = parse_int_arg(argv[5]);
  // OpenMP initialization
  omp_set_num_threads(t);
#else
  t = 0;
  iteration = parse_int_arg(argv[4]);
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

  // Root process generates the seeds for each block of the A and B matrices
  if (rank == 0) {
    seeds_per_dim = malloc(sizeof(*seeds_per_dim) * (dims[0] + dims[1]));
    seeds = malloc(sizeof(*seeds) * p * 2);
    if (seeds == NULL || seeds_per_dim == NULL) {
      perror("Error allocating seeds array");
      MPI_Abort(topology_comm, EXIT_FAILURE);
    }
    srand(SEED);
    // Generating real seeds for matrices generation
    for (int i = 0; i < dims[0] + dims[1]; i++)
      seeds_per_dim[i] = rand();

    // What seeds each processes receives:
    //  process i receives 2 * i for A and 2 * i + 1 for B
    for (int i = 0; i < p; i++) {
      int i_coords[2];
      MPI_Cart_coords(topology_comm, i, 2, i_coords);
      seeds[2 * i] = seeds_per_dim[i_coords[0]];
      seeds[2 * i + 1] = seeds_per_dim[dims[0] + i_coords[1]];
    }
  }

  // Sending the seeds to the processes (2 ints for each process); saving into local_seeds
  sendbuf = rank == 0 ? seeds : NULL;
  MPI_Scatter(sendbuf, 2, MPI_INT, local_seeds, 2, MPI_INT, 0, topology_comm);

  MPI_Barrier(topology_comm);
  stats.first_communication_time = MPI_Wtime() - start_time; // First communication time

  start_time = MPI_Wtime();

#ifndef DEBUG
  gen_type = RANDOM;
#else
  gen_type = INDEX;
#endif

  // Each process allocates the local matrix with the seed received from root
  local_a = matrix_init(n_rows, k, gen_type, local_seeds[0]);
  local_b = matrix_init(k, n_cols, gen_type, local_seeds[1]);
  // Needed as the parallel multiplication routine expects B to be transposed
  local_b_t = matrix_init(n_cols, k, ZERO, 0);
  c = matrix_init(m, n, ZERO, 0);
  if (local_a == NULL || local_b == NULL || local_b_t == NULL || c == NULL) {
    perror("Error allocating local matrices");
    MPI_Abort(topology_comm, EXIT_FAILURE);
  }

  MPI_Barrier(topology_comm);
  stats.generation_time = MPI_Wtime() - start_time; // Generation time

  start_time = MPI_Wtime();

  // Transposing the B matrix
  matrix_transpose(local_b, local_b_t, k, n_cols);

  // Parallel computation
  matrix_parallel_mult(local_a, local_b_t, c, n_rows, n_cols, k, n, start_rows, start_cols);

  MPI_Barrier(topology_comm);
  stats.parallel_time = MPI_Wtime() - start_time; // Parallel computation time

  start_time = MPI_Wtime();

  // Gathering results
  MPI_Allreduce(MPI_IN_PLACE, c, m * n, MPI_FLOAT, MPI_SUM, topology_comm);

  MPI_Barrier(topology_comm);                                 // Waiting for root process to finish ordering
  stats.second_communication_time = MPI_Wtime() - start_time; // Second communication time

  // MPI Cleanup
  MPI_Comm_free(&topology_comm);
  MPI_Comm_free(&temp_comm);
  MPI_Finalize();

  // Program has effectively finished
  // What follows is additional tasks for the serial check and stats writing
  // These tasks will only be executed by root process
  if (rank != 0)
    goto close;

  // Initialize global matrices
  a = matrix_init(m, k, ZERO, 0);
  b = matrix_init(k, n, ZERO, 0);
  c_serial = matrix_init(m, n, ZERO, 0);
  if (a == NULL || b == NULL || c_serial == NULL) {
    perror("Error allocating matrices for serial check");
    return EXIT_FAILURE;
  }

  // Generating A matrix
  offset = 0;
  for (int i = 0; i < dims[0]; i++) {
    int i_start_rows, i_end_rows, i_rows;
    calculate_start_end(m, dims[0], i, &i_start_rows, &i_end_rows);
    i_rows = i_end_rows - i_start_rows;
    srand(seeds_per_dim[i]);
    for (int j = 0; j < i_rows * k; j++)
      a[j + offset * k] = gen_type == INDEX ? j : (float)rand() / RAND_MAX;
    offset += i_rows;
  }

  // Generating B matrix
  offset = 0;
  for (int i = 0; i < dims[1]; i++) {
    int i_start_cols, i_end_cols, i_cols;
    calculate_start_end(n, dims[1], i, &i_start_cols, &i_end_cols);
    i_cols = i_end_cols - i_start_cols;
    srand(seeds_per_dim[dims[0] + i]);
    for (int l = 0; l < k; l++) {
      for (int j = 0; j < i_cols; j++) {
        b[j + i_start_cols + l * n] = gen_type == INDEX ? offset++ : (float)rand() / RAND_MAX;
      }
    }
    offset = 0;
  }

  if (root_tasks(a, b, c, c_serial, m, n, k, &stats, MPIv2, iteration) != 0)
    return EXIT_FAILURE;

close:
  // Cleanup
  if (rank == 0) {
    free(a);
    free(b);
    free(c_serial);
    free(seeds);
    free(seeds_per_dim);
  }
  free(local_a);
  free(local_b);
  free(local_b_t);
  free(c);
  return EXIT_SUCCESS;
}
