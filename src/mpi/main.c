#include <errno.h>
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

// TODO: remove as it is an unusable implementation

// Read portion of the matrices
int read_block_matrices(char *folder, int m, int n, int k, int start_rows, int end_rows, int start_cols, int end_cols,
                        float *a, float *b) {
  char *a_path, *b_path;
  FILE *a_fp, *b_fp;

  a_path = create_file_path(folder, "a.bin");
  b_path = create_file_path(folder, "b.bin");
  if (a_path == NULL || b_path == NULL)
    return -1;

  a_fp = fopen(a_path, "r");
  b_fp = fopen(b_path, "r");
  if (a_fp == NULL || b_fp == NULL) {
    perror("Error opening matrix files");
    return -1;
  }

  // Read A matrix rows of this process
  for (int i = start_rows; i < end_rows; i++) {
    // Calculating where the row is in the file
    int offset_file = i * k;
    // Calculating where the row should be placed in the local array
    int offset_array = (i - start_rows) * k;
    // Moving to the current row in the file
    fseek(a_fp, offset_file * sizeof(*a), SEEK_SET);
    // Reading the entire row from the file
    fread(a + offset_array, sizeof(*a), k, a_fp);
  }

  // Read B matrix cols of this process
  for (int i = 0; i < k; i++) {
    // Calculating where the row starts in the file
    int offset_file = i * n + start_cols;
    // Calculating where the cols should be placed in the local array
    int offset_array = i * (end_cols - start_cols);
    // Moving to the current col in the file
    fseek(b_fp, offset_file * sizeof(*b), SEEK_SET);
    // Reading the cols from the file
    fread(b + offset_array, sizeof(*b), end_cols - start_cols, b_fp);
  }

  // Freeing memory
  fclose(a_fp);
  fclose(b_fp);
  free(a_path);
  free(b_path);

  return 0;
}

int main(int argc, char **argv) {
  // Variable Declaration
  int m = 0, n = 0, k = 0;                               // Matrix dimension (from arguments)
  int i, j, l;                                           // Loop Control Variables
  int start_cols, start_rows, end_cols, end_rows;        // Offsets based on topology
  float *a, *b, *b_t, *c, *c_file;                       // Matrices
  int p, t;                                              // Number of processes and threads
  int row_color, col_color;                              // Row and column communicator discriminator
  int rank, row_rank, col_rank;                          // Ranks in 2D Topology, row and column comm
  int c_size;                                            // Size of the C matrix (depends on row and col rank)
  MPI_Comm topology_comm, temp_comm, row_comm, col_comm; // Custom communicator
  int dims[2], periods[2], coords[2];                    // Topology dimensions and coordinates
  char *folder, *c_path;                                 // Path for the specific matrix
  FILE *c_fp;                                            // Files of the matrices
  int *col_recv_counts, *col_offsets;                    // Gatherv parameters for column communicator
  void *sendbuf, *recvbuf;                               // Buffer used to determine how the Gatherv will work

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

#ifdef _OPENMP
  t = parse_int_arg(argv[4]);
  // OpenMP initialization
  omp_set_num_threads(t);
#else
  t = 0;
#endif

  // MPI Initialization
  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &temp_comm);
  double start_time = MPI_Wtime();
  // Getting basics information; using a temp communicator even though a new comm will be created by the topology
  MPI_Comm_rank(temp_comm, &rank);
  MPI_Comm_size(temp_comm, &p);

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
  // Row and Column Communicator
  row_color = coords[0];                          // Processes in the same row are in the same communicator
  col_color = coords[1] == 0 ? 1 : MPI_UNDEFINED; // Processes in the first column are in this communicator
  MPI_Comm_split(topology_comm, row_color, rank, &row_comm);
  MPI_Comm_split(topology_comm, col_color, rank, &col_comm);
  // Rank in the new communicators
  MPI_Comm_rank(row_comm, &row_rank);   // Getting the rank in the row topology
  col_rank = -1;                        // Default value for processes not in the first column
  if (col_comm != MPI_COMM_NULL)        // If the process is in the first column
    MPI_Comm_rank(col_comm, &col_rank); // Getting the rank in the column topology

  // Calculating offsets and number of items assigned to each process
  // Taking into account odd values and fair distribution
  calculate_start_end(m, dims[0], coords[0], &start_rows, &end_rows);
  calculate_start_end(n, dims[1], coords[1], &start_cols, &end_cols);
  int n_rows = end_rows - start_rows; // Matrix A rows to load
  int n_cols = end_cols - start_cols; // Matrix B cols to load

  // Reading matrices

  // Allocating matrices
  a = malloc(sizeof(*a) * (end_rows - start_rows) * k);
  b = malloc(sizeof(*b) * (end_cols - start_cols) * k);
  b_t = malloc(sizeof(*b_t) * (end_cols - start_cols) * k);
  if (a == NULL || b == NULL || b_t == NULL) {
    perror("Error allocating matrices");
    return -1;
  }

  // Opening folder path
  folder = create_folder_path(m, n, k);
  if (folder == NULL)
    MPI_Abort(topology_comm, -1);

  // Read matrices
  if (read_block_matrices(folder, m, n, k, start_rows, end_rows, start_cols, end_cols, a, b) != 0)
    MPI_Abort(topology_comm, -1);

  // Transposed B matrix (better read access)
  matrix_transpose(b, b_t, k, end_cols - start_cols);

  // Calculating C matrix size based on the process type
  c_size = col_rank == 0 ? m * n : n_rows * n; // Column root allocates the entire C matrix

  // Allocating C matrix
  c = malloc(sizeof(*c) * c_size);
  if (c == NULL) {
    fprintf(stderr, "Error allocating matrix C (size %d): %s\n", c_size, strerror(errno));
    MPI_Abort(topology_comm, -1);
  }
  // Resetting C matrix memory to 0
  memset(c, 0, sizeof(*c) * c_size);

  // Perform local multiplication for a sub-matrix in a serial way (or using OpenMP if enabled)
  // Using collapse(2) instead of 3 to optimize write access
  // (with collapse 3, the tmp variable cannot be used)
#pragma omp parallel for private(i, j, l) shared(a, b, c) collapse(2)
  // Parallel Computation
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      float tmp = 0.0;
#pragma omp simd
      for (l = 0; l < k; l++) {
        tmp += a[i * k + l] * b[j * k + l];
      }
      c[i * n_cols + j + start_cols] += tmp;
    }
  }

  // Collect data:
  // 1. Processes in each row communicator send the data to each root row process
  // 2. Processes in the column communicator send the data to the root column process

  // Row communication
  sendbuf = row_rank == 0 ? MPI_IN_PLACE : c; // Root node in the row uses the same buffer
  recvbuf = row_rank == 0 ? c : NULL;         // Root receives in C
  // Summing the contributions of every process in the row
  MPI_Reduce(sendbuf, recvbuf, n * n_rows, MPI_FLOAT, MPI_SUM, 0, row_comm);

  // Column communication

  // Only the processes in the column communicator have to do something
  if (col_comm != MPI_COMM_NULL) {
    // Allocating memory for the Gatherv parameters
    col_recv_counts = malloc(sizeof(*col_recv_counts) * dims[0]);
    col_offsets = malloc(sizeof(*col_offsets) * dims[0]);
    if (col_recv_counts == NULL || col_offsets == NULL) {
      perror("Error allocating offsets and receiving counts for cols");
      MPI_Abort(topology_comm, -1);
    }
    // Iterating through each process
    for (int i = 0; i < p; i++) {
      // Getting the coords of that process in the 2D topology
      int i_coords[2];
      MPI_Cart_coords(topology_comm, i, 2, i_coords);
      if (i_coords[1] != 0) // not in first column, skipping
        continue;
      // Calculating assigned rows and cols
      int i_start_rows, i_end_rows;
      calculate_start_end(m, dims[0], i_coords[0], &i_start_rows, &i_end_rows);
      // Assigning the counts and offsets
      col_recv_counts[i_coords[0]] = (i_end_rows - i_start_rows) * n;
      col_offsets[i_coords[0]] = i_start_rows * n;
    }

    // Gathering results from the other processes in the column
    sendbuf = col_rank == 0 ? MPI_IN_PLACE : c; // Root node in the column uses the same buffer
    recvbuf = col_rank == 0 ? c : NULL;         // Root receives in C
    MPI_Gatherv(sendbuf, n * n_rows, MPI_FLOAT, recvbuf, col_recv_counts, col_offsets, MPI_FLOAT, 0, col_comm);

    // Rank 0 does additional work
    if (col_rank == 0) {
      // Computation has finished; what follows is checking the results and writing stats to output file
      double end_time = MPI_Wtime();

      // Opening C file for result checking
      c_path = create_file_path(folder, "c.bin");
      if (c_path == NULL)
        MPI_Abort(topology_comm, -1);
      c_fp = fopen(c_path, "r");
      if (c_fp == NULL)
        MPI_Abort(topology_comm, -1);

      // Allocating space necessary for reading the C matrix from file
      c_file = malloc(sizeof(*c_file) * m * n);
      if (c_file == NULL) {
        perror("Error allocating C matrix (file)");
        MPI_Abort(topology_comm, -1);
      }
      // Reading matrix C from file
      fread(c_file, sizeof(*c_file), m * n, c_fp);

      // Calculating the error
      float error = calculate_error(c, c_file, m, n);

#ifdef _OPENMP
      char *output_name = "mpi-omp.csv";
#else
      char *output_name = "mpi.csv";
#endif

      // Writing stats to specific file (mpi.csv or mpi-omp.csv, based on variant used)
      if (write_stats(output_name, m, n, k, p, t, end_time - start_time, error))
        MPI_Abort(topology_comm, -1);

#ifdef DEBUG
        // Print final matrix in debug mode
        // matrix_print(c, m, n);
#endif

      // Free memory used by the root column process
      free(c_file);
      free(c_path);
      fclose(c_fp);
    }
    // Free memory of the column communicator
    free(col_recv_counts);
    free(col_offsets);
    MPI_Comm_free(&col_comm);
  }

  // Free
  free(folder);
  free(a);
  free(b);
  free(c);
  MPI_Comm_free(&row_comm);
  MPI_Comm_free(&topology_comm);
  MPI_Comm_free(&temp_comm);
  MPI_Finalize();
  return 0;
}
