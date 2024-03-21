#ifdef _OPENMP
#include <omp.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "../common/matrix.h"
#include "../common/utils.h"

int main(int argc, char **argv) {
  // Variable definition
  int m, n, k, p;                          // From CLI, matrices size and number of threads
  int i, j, l;                             // Loop control variables
  double start_time = 0.0;                 // Start time of computation
  double end_time = 0.0;                   // End time of computation
  char *folder, *a_path, *b_path, *c_path; // Paths
  FILE *a_fp, *b_fp, *c_fp;                // Matrices file
  float *a, *b, *c, *c_file;               // Matrices

  // Parse arguments from command line
  if (argc != 5) {
    printf("Usage: %s <m> <n> <k> <t>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  p = parse_int_arg(argv[4]);

  // Different behavior based on if it's running with OpenMP
#ifdef _OPENMP
  omp_set_num_threads(p);       // Set threads based on argument from CLI
  start_time = omp_get_wtime(); // Get time from OpenMP
#else
  // Get time with a system-call when OpenMP is not being used
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start_time = tv.tv_sec + tv.tv_usec / 1000000.0; // Convert to the same format used by omp_get_wtime
#endif

  // Calculate folder path of the matrices
  folder = create_folder_path(m, n, k);
  if (folder == NULL)
    return -1;
  // Get file paths for the matrices
  a_path = create_file_path(folder, "a.bin");
  b_path = create_file_path(folder, "b.bin");
  c_path = create_file_path(folder, "c.bin");
  if (a_path == NULL || b_path == NULL || c_path == NULL)
    return -1;

  // Open the files
  a_fp = fopen(a_path, "r");
  b_fp = fopen(b_path, "r");
  c_fp = fopen(c_path, "r");
  if (a_fp == NULL || b_fp == NULL || c_fp == NULL) {
    perror("Error opening files");
    return -1;
  }

  // Allocating matrices
  a = malloc(sizeof(*a) * m * k);
  b = malloc(sizeof(*b) * n * k);
  c = malloc(sizeof(*c) * m * m);           // Calculated C matrix
  c_file = malloc(sizeof(*c_file) * m * m); // C matrix read from file
  if (a == NULL || b == NULL || c == NULL || c_file == NULL) {
    perror("Error allocating memory for matrices");
    return -1;
  }
  // Reading the matrices from the file
  fread(a, sizeof(*a), m * k, a_fp);
  fread(b, sizeof(*b), n * k, b_fp);
  fread(c_file, sizeof(*c_file), n * m, c_fp);

  // Parallel Computation
#pragma omp parallel for private(i, j, l) shared(a, b, c) collapse(2)
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
#pragma omp simd
      for (l = 0; l < k; l++)
        c[i * n + j] += a[i * k + l] * b[l * n + j];

  // Calculating the error
  float error = calculate_error(c, c_file, m, n);

  // Just like before, calculate the time using OpenMP or a system-call
#ifdef _OPENMP
  end_time = omp_get_wtime();
#else
  gettimeofday(&tv, NULL);
  end_time = tv.tv_sec + tv.tv_usec / 1000000.0;
#endif

  // Writing stats to file
  if (write_stats("output/omp.csv", m, n, k, p, end_time - start_time, error))
    exit(EXIT_FAILURE);
#ifdef DEBUG
  // Print final matrix (in debug mode only)
  matrix_print(c, m, n);
#endif
  return 0;
}
