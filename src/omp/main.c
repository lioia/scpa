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
  int m, n, k, t, iteration;         // From CLI, matrices size, number of threads and iteration
  double start_time;                 // Start and delta times for generation and parallel
  float *a, *b, *b_t, *c, *c_serial; // Matrices
  enum gen_type_t type;              // Generation type
  stats_t stats;                     // Stats

  // Parse arguments from command line
  if (argc != 6) {
    printf("Usage: %s <m> <n> <k> <t> <iteration>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  t = parse_int_arg(argv[4]);
  iteration = parse_int_arg(argv[5]);
  memset(&stats, 0, sizeof(stats));
  stats.threads = t;

#ifdef _OPENMP
  omp_set_num_threads(t); // Set threads based on argument from CLI
#else
  printf("[WARN] Not running OpenMP\n");
#endif
  start_time = get_time();

#ifndef DEBUG
  type = RANDOM;
#else
  type = INDEX;
#endif

  // Allocating matrices
  a = matrix_init(m, k, type, SEED);
  b = matrix_init(k, n, type, SEED + 1);
  b_t = matrix_init(k, n, ZERO, 0);
  c = matrix_init(m, n, ZERO, 0);        // Initial C matrix
  c_serial = matrix_init(m, n, ZERO, 0); // Serial C matrix
  if (a == NULL || b == NULL || b_t == NULL || c == NULL || c_serial == NULL) {
    perror("Error creating matrices");
    return EXIT_FAILURE;
  }

  // Transpose (better cache read access)
  matrix_transpose(b, b_t, k, n);

  stats.generation_time = get_time() - start_time;

  // Calculate using OpenMP
  start_time = get_time();
  matrix_parallel_mult(a, b_t, c, m, n, k, n, 0, 0);
  stats.parallel_time = get_time() - start_time;

  if (root_tasks(a, b, c, c_serial, m, n, k, &stats, OMP, iteration) != 0)
    return EXIT_FAILURE;

  // Cleanup
  free(a);
  free(b);
  free(b_t);
  free(c);
  free(c_serial);
  return EXIT_SUCCESS;
}
