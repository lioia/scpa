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
// inlined as most of the time is just a single function call
double get_time() {
#ifdef _OPENMP
  return omp_get_wtime(); // Get time from OpenMP
#else
  // Get time with a system-call when OpenMP is not being used
  struct timeval tv;
  gettimeofday(&tv, NULL);
  // Convert to the same format used by omp_get_wtime
  return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
}

int main(int argc, char **argv) {
  // Variable definition
  int m, n, k, t;                    // From CLI, matrices size and number of threads
  double start_time = 0.0;           // Start time of checkpoint
  double g_time = 0.0;               // Delta time for generation
  double p_time = 0.0;               // Delta time for parallel computation
  double s_time = 0.0;               // Delta time for serial computation
  double error = 0.0;                // Difference between serial and parallel computation
  float *a, *b, *b_t, *c, *c_serial; // Matrices
  enum gen_type_t type = RANDOM;     // Generation type

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
  a = matrix_init(m, k, type, 0);
  b = matrix_init(k, n, type, 1);
  b_t = malloc(sizeof(*b_t) * n * k);
  c = matrix_init(m, n, type, 2);        // Initial C matrix
  c_serial = matrix_init(m, n, type, 2); // Initial C matrix, used for serial multiplication
  if (a == NULL || b == NULL || b_t == NULL || c == NULL) {
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
  if (write_stats("omp.csv", m, n, k, 0, t, error, g_time, p_time, s_time, 0, 0))
    exit(EXIT_FAILURE);

#ifdef DEBUG
  // Print final matrix (in debug mode only)
  matrix_print(c, m, n);
#endif

  // Free memory
  free(a);
  free(b);
  free(b_t);
  free(c);
  free(c_serial);
  return 0;
}
