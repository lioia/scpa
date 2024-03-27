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

// NOTE: this implementation is the variant with 2D block distribution on C (not cyclic)

int main(int argc, char **argv) {
  // Variable Declaration
  int m = 0, n = 0, k = 0;                         // Matrix dimension (from arguments)
  int i, j, l;                                     // Loop control variables
  int start_cols, start_rows, end_cols, end_rows;  // Offsets based on topology
  float *local_a, *local_b, *local_c, *c, *c_file; // Matrices
  int rank, p;                                     // Process rank and number of processes
  MPI_Comm topologyComm, tempComm;                 // Custom communicator
  int dims[2], periods[2];                         // Topology dimensions
  int coords[2];                                   // Coordinates in topology
  char *folder, *a_path, *b_path, *c_path;         // Path for the specific matrix
  FILE *a_fp, *b_fp, *c_fp;                        // Files of the matrices

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
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
#ifdef _OPENMP
  int t = parse_int_arg(argv[4]);
  omp_set_num_threads(t);
#endif
  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &tempComm);
  double start_time = MPI_Wtime();
  // Getting basics information; using a temp communicator even though a new comm will be created by the topology
  MPI_Comm_rank(tempComm, &rank);
  MPI_Comm_size(tempComm, &p);

#ifdef _OPENMP
  if (rank == 0)
    printf("Running with OpenMP\n");
#endif

  // Topology
  dims[0] = dims[1] = 0;                                         // Default assignment
  periods[0] = periods[1] = 0;                                   // No wrapping
  MPI_Dims_create(p, 2, dims);                                   // Create dimensions
  MPI_Cart_create(tempComm, 2, dims, periods, 0, &topologyComm); // Create topology
  MPI_Cart_coords(topologyComm, rank, 2, coords);                // Get coords in topology

  // Calculating offsets and number of items assigned to each process
  // Taking into account odd values and fair distribution
  calculate_start_end(m, dims[0], coords[0], 0, &start_rows, &end_rows);
  calculate_start_end(n, dims[1], coords[1], coords[0] * dims[0], &start_cols, &end_cols);
  int n_cols = end_cols - start_cols;
  int n_rows = end_rows - start_rows;

  // Allocating only the necessary space the matrices
  local_a = malloc(sizeof(*local_a) * n_rows * k);
  local_b = malloc(sizeof(*local_b) * n_cols * k);
  local_c = malloc(sizeof(*local_c) * n_rows * n);
  if (local_a == NULL || local_b == NULL || local_c == NULL) {
    perror("Error allocating matrices");
    MPI_Abort(topologyComm, -1);
  }
  memset(local_c, 0, sizeof(*local_c) * n_rows * n);
  // Creating the path for the matrices
  folder = create_folder_path(m, n, k);
  if (folder == NULL)
    MPI_Abort(topologyComm, -1);
  a_path = create_file_path(folder, "a.bin");
  b_path = create_file_path(folder, "b.bin");
  c_path = create_file_path(folder, "c.bin");
  if (a_path == NULL || b_path == NULL || c_path == NULL)
    MPI_Abort(topologyComm, -1);

  a_fp = fopen(a_path, "r");
  b_fp = fopen(b_path, "r");
  c_fp = fopen(c_path, "r");
  if (a_fp == NULL || b_fp == NULL || c_fp == NULL) {
    perror("Error opening matrix files");
    MPI_Abort(topologyComm, -1);
  }

  // Read A matrix rows of this process
  for (size_t i = start_rows; i < end_rows; i++) {
    // Calculating where the row is in the file
    int row_pos_in_file = i * k;
    // Calculating where the row should be placed in the local array
    int row_pos_in_array = (i - start_rows) * k;
    // Moving to the current row in the file
    fseek(a_fp, row_pos_in_file * sizeof(*local_a), SEEK_SET);
    // Reading the entire row from the file
    fread(local_a + row_pos_in_array, sizeof(*local_a), k, a_fp);
  }

  // Read B matrix cols of this process
  int curr = 0; // Position in local array
  for (size_t j = 0; j < k; j++) {
    for (size_t i = start_cols; i < end_cols; i++) {
      // Calculating where the value is in the file
      int pos_in_file = j * n + i;
      // Moving to the value in the file
      fseek(b_fp, pos_in_file * sizeof(*local_b), SEEK_SET);
      // Reading the value into a temp variable
      float value = 0.0;
      fread(&value, sizeof(value), 1, b_fp);
      // Saving the value in the local array
      local_b[curr++] = value;
    }
  }

  // Perform local multiplication in a serial way (or using OpenMP if enabled)
#pragma omp parallel for private(i, j, l) shared(local_a, local_b, local_c) collapse(2)
  for (i = 0; i < n_rows; i++)
    for (j = 0; j < n_cols; j++)
      for (l = 0; l < k; l++)
        local_c[i * n + j + start_cols] += local_a[i * k + l] * local_b[l * n_cols + j];

  // Collect data:
  // 1. Each process sends the data to the first process in the row
  // 2. Each first process in the row, sends the data to the first process in the column

  // 1. Each process sends the data to the first process in the row

  // Creating sub-partitions on topology; a sub-partition for every row
  int row_sub_coords[2] = {0, 1}; // [0] is for row (varying); [1] is for column (fixed)
  MPI_Comm row_comm;
  MPI_Cart_sub(topologyComm, row_sub_coords, &row_comm);
  // Getting the rank
  int row_rank;
  MPI_Comm_rank(row_comm, &row_rank);
  // Applying a reduce operation to each row
  const void *send_buf = MPI_IN_PLACE; // Rank 0 sums to the same matrix
  if (row_rank != 0)
    send_buf = local_c; // Every other processes to the local matrix
  // Collecting results for every row
  MPI_Reduce(send_buf, local_c, n * n_rows, MPI_FLOAT, MPI_SUM, 0, row_comm);

  // 2. Each first process in the row, sends the data to the first process in the column

  // Array containing the data each process has to send and the offsets
  int *col_recv_counts = malloc(sizeof(*col_recv_counts) * dims[0]);
  int *col_offsets = malloc(sizeof(*col_offsets) * dims[0]);
  if (col_recv_counts == NULL || col_offsets == NULL) {
    perror("Error allocating column group");
    MPI_Abort(topologyComm, -1);
  }

  // Calculating whether the process i is in the first column and how many data has to sends
  int col_idx = 0;
  for (size_t i = 0; i < p; i++) {
    // Getting the coordinates for this process
    int i_coords[2];
    MPI_Cart_coords(topologyComm, i, 2, i_coords);
    if (i_coords[1] != 0) // not in the first column, so skipping
      continue;
    // Calculating offsets and size
    int start, end;
    calculate_start_end(m, dims[0], i_coords[0], 0, &start, &end);
    col_recv_counts[col_idx] = (end - start) * n;
    col_offsets[col_idx++] = start * n;
  }

  // Creating new communicator for only the processes in the first column of the topology
  int color = MPI_UNDEFINED; // no subgroup
  if (coords[1] == 0)        // subgroup only for processes in the first column
    color = 1;
  MPI_Comm col_comm;
  MPI_Comm_split(topologyComm, color, rank, &col_comm); // topology communicator split based on column
  if (col_comm != MPI_COMM_NULL) {
    // Getting the rank in the new communicator
    int col_rank;
    MPI_Comm_rank(col_comm, &col_rank);
    if (col_rank == 0) {
      // Only rank 0 allocates the necessary memory for the resulting c and the c loaded from file
      c = malloc(sizeof(*c) * m * n);
      c_file = malloc(sizeof(*c_file) * m * n);
      if (c == NULL || c_file == NULL) {
        perror("Error allocating C matrices");
        MPI_Abort(topologyComm, -1);
      }
      // Reading matrix C from file
      fread(c_file, sizeof(*c_file), m * n, c_fp);
    }
    // Gathering results from the other processes in the column
    MPI_Gatherv(local_c, n * n_rows, MPI_FLOAT, c, col_recv_counts, col_offsets, MPI_FLOAT, 0, col_comm);
    // Rank 0 checks the result and prints the time
    if (col_rank == 0) {
      // Calculating the error
      float error = calculate_error(c, c_file, m, n);
      double end_time = MPI_Wtime();

#ifdef _OPENMP
      char *output_name = "mpi-omp.csv";
      int x_value = p * t;
#else
      char *output_name = "mpi.csv";
      int x_value = p;
#endif

      if (write_stats(output_name, m, n, k, x_value, end_time - start_time, error))
        MPI_Abort(topologyComm, -1);

#ifdef DEBUG
      // Print final matrix in debug mode
      matrix_print(c, m, n);
#endif
      free(c_file);
      free(c);
    }
    MPI_Comm_free(&col_comm);
  }

  // Free
  free(col_recv_counts);
  free(col_offsets);
  fclose(a_fp);
  fclose(b_fp);
  fclose(c_fp);
  free(a_path);
  free(b_path);
  free(c_path);
  free(folder);
  free(local_a);
  free(local_b);
  free(local_c);
  MPI_Comm_free(&row_comm);
  MPI_Comm_free(&topologyComm);
  MPI_Comm_free(&tempComm);
  MPI_Finalize();
  return 0;
}
