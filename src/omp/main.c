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

// Helper function to calculate the time using OpenMP or gettimeofday
#ifdef _OPENMP
#define get_time() omp_get_wtime()
#else
#define get_time() get_time_syscall()
#endif

int main(int argc, char **argv) {
  // Variable definition
  int m, n, k, t;                            // From CLI, matrices size and number of threads
  double start_time, g_time, p_time, s_time; // Start and delta times for generation, parallel and serial computation
  double error;                              // Difference between serial and parallel computation
  float *a, *b, *b_t, *c, *c_serial;         // Matrices
  enum gen_type_t type = RANDOM;             // Generation type
  FILE *stats_fp;                            // Stats file

  // Parse arguments from command line
  if (argc != 5) {
    printf("Usage: %s <m> <n> <k> <t>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  t = parse_int_arg(argv[4]);

#ifdef _OPENMP
  omp_set_num_threads(t); // Set threads based on argument from CLI
#else
  printf("[WARN] Not running OpenMP\n");
#endif
  start_time = get_time();

#ifdef DEBUG
  type = INDEX;
#endif

  // Allocating matrices
  a = matrix_init(m, k, type, SEED);
  b = matrix_init(k, n, type, SEED + 1);
  b_t = malloc(sizeof(*b_t) * n * k);
  c = matrix_init(m, n, ZERO, 0);        // Initial C matrix
  c_serial = matrix_init(m, n, ZERO, 0); // Initial C matrix
  if (a == NULL || b == NULL || b_t == NULL || c == NULL || c_serial) {
    perror("Error creating matrices");
    return -1;
  }

  // Transpose (better cache read access)
  matrix_transpose(b, b_t, k, n);

  g_time = get_time() - start_time;

  // Calculate using OpenMP
  start_time = get_time();
  matrix_parallel_mult(a, b_t, c, m, n, k, n, 0, 0);
  p_time = get_time() - start_time;

  // Calculate using serial implementation
  start_time = get_time();
  matrix_serial_mult(a, b, c_serial, m, n, k);
  s_time = get_time() - start_time;

  // Calculating the error
  error = calculate_error(c, c_serial, m, n);

  // Writing stats to file

  stats_fp = open_stats_file("omp.csv");
  if (stats_fp == NULL) {
    perror("Error opening stats file");
    return EXIT_FAILURE;
  }
  fprintf(stats_fp, "%d,%d,%d,0,%d,0,%f,0,0,%f,%f\n", m, n, k, t, p_time, s_time, error);

#ifdef DEBUG
  // Print final matrix (in debug mode only)
  matrix_print(c, m, n);
#endif

  // Cleanup
  free(a);
  free(b);
  free(b_t);
  free(c);
  free(c_serial);
  fclose(stats_fp);
  return 0;
}
