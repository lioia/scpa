#ifdef _OPENMP
#include <omp.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/matrix.h"
#include "../common/utils.h"

// Read matrices from file
int read_matrices(float *a, float *b, float *c, int m, int n, int k) {
  // Create folder path, based on matrix sizes
  char *folder = create_folder_path(m, n, k);
  if (folder == NULL)
    return -1;
  // Create file path for matrix A, B and C
  char *a_path = create_file_path(folder, "a.bin");
  char *b_path = create_file_path(folder, "b.bin");
  char *c_path = create_file_path(folder, "c.bin");
  if (a_path == NULL || b_path == NULL || c_path == NULL)
    return -1;
  // Open matrix file in read mode
  FILE *a_fp = fopen(a_path, "r");
  FILE *b_fp = fopen(b_path, "r");
  FILE *c_fp = fopen(c_path, "r");
  if (a_fp == NULL || b_fp == NULL || c_fp == NULL) {
    perror("Error opening files");
    return -1;
  }
  // Read matrices from file
  fread(a, sizeof(*a), m * k, a_fp);
  matrix_read_transposed(b, b_fp, n, k, 0, n);
  fread(c, sizeof(*c), n * m, c_fp);
  // Closing file
  fclose(a_fp);
  fclose(b_fp);
  fclose(c_fp);
  // Freeing unused memory
  free(a_path);
  free(b_path);
  free(c_path);
  free(folder);
  return 0;
}

int main(int argc, char **argv) {
  // Variable definition
  int m, n, k, t;            // From CLI, matrices size and number of threads
  double start_time = 0.0;   // Start time of computation
  double end_time = 0.0;     // End time of computation
  float *a, *b, *c, *c_file; // Matrices

  // Parse arguments from command line
  if (argc != 5) {
    printf("Usage: %s <m> <n> <k> <t>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  t = parse_int_arg(argv[4]);

  // Different behavior based on if it's running with OpenMP
#ifdef _OPENMP
  omp_set_num_threads(t);       // Set threads based on argument from CLI
  start_time = omp_get_wtime(); // Get time from OpenMP
#else
  // Get time with a system-call when OpenMP is not being used
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start_time = tv.tv_sec + tv.tv_usec / 1000000.0; // Convert to the same format used by omp_get_wtime
  printf("[WARN] Not running OpenMP\n");
#endif

  // Allocating matrices
  a = malloc(sizeof(*a) * m * k);
  b = malloc(sizeof(*b) * n * k);
  c = malloc(sizeof(*c) * m * n);           // Calculated C matrix
  c_file = malloc(sizeof(*c_file) * m * n); // C matrix read from file
  if (a == NULL || b == NULL || c == NULL || c_file == NULL) {
    perror("Error allocating memory for matrices");
    return -1;
  }

  // Read matrices from files
  if (read_matrices(a, b, c_file, m, n, k) != 0)
    return -1;
  memset(c, 0, sizeof(*c) * m * n);

  // Calculate using OpenMP
  matrix_parallel_mult(a, b, c, m, n, k, 0, 0);

// Just like before, calculate the time using OpenMP or a system-call
#ifdef _OPENMP
  end_time = omp_get_wtime();
#else
  gettimeofday(&tv, NULL);
  end_time = tv.tv_sec + tv.tv_usec / 1000000.0;
#endif

  // Calculating the error
  float error = calculate_error(c, c_file, m, n);

  // Writing stats to file
  if (write_stats("omp.csv", m, n, k, 0, t, end_time - start_time, error))
    exit(EXIT_FAILURE);

#ifdef DEBUG
  // Print final matrix (in debug mode only)
  matrix_print(c, m, n);
#endif

  // Free memory
  free(a);
  free(b);
  free(c);
  free(c_file);
  return 0;
}
