#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "matrix.h"
#include "utils.h"

// Return parsed int from arg; exits otherwise
int parse_int_arg(char *arg) {
  // Parse to an int
  int val = strtol(arg, NULL, 10);
  // Check errors
  if ((val == INT_MAX || val == INT_MIN) && errno == ERANGE) {
    fprintf(stderr, "Error parsing m (%s): out of range", arg);
    exit(EXIT_FAILURE);
  }
  return val;
}

float frobenius_norm(int m, int n, float *matrix) {
  float sum = 0.0;
  for (int i = 0; i < m * n; i++)
    sum += matrix[i] * matrix[i];
  return (float)sqrt(sum);
}

float calculate_error(float *c, float *c_serial, int m, int n) {
  float *diff = malloc(sizeof(*diff) * m * n);
  if (diff == NULL) {
    perror("Error allocating matrix diff");
    return -1;
  }

  // Calculate the difference matrix D = C - C_serial and store in diff
  for (int i = 0; i < m * n; i++)
    diff[i] = c[i] - c_serial[i];

  // Calculate the Frobenius norms
  float norm_diff = frobenius_norm(m, n, diff);
  float norm_c = frobenius_norm(m, n, c);

  free(diff);
  return norm_diff / norm_c;
}

double get_time_syscall() {
  // Get time with a system-call
  struct timeval tv;
  gettimeofday(&tv, NULL);
  // Convert to the same format used by omp_get_wtime and MPI_Wtime
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int root_tasks(float *a, float *b, float *c, float *c_serial, int m, int n, int k, stats_t *stats, version_t version) {
  double start_time, serial_time;
  float error;
  char *stats_path;
  FILE *stats_fp;

#ifdef DEBUG
  // matrix_print(c, m, n);
#endif /* ifdef DEBUG */

  start_time = get_time_syscall();
  matrix_serial_mult(a, b, c_serial, m, n, k);
  serial_time = get_time_syscall() - start_time; // Serial computation

  // Calculate the error
  error = calculate_error(c, c_serial, m, n);

  // Determine stats filename
  if (version == OMP) {
    stats_path = "output/omp.csv";
  } else if (version == MPIv1) {
#ifdef _OPENMP
    stats_path = "output/mpi-omp-v1.csv";
#else
    stats_path = "output/mpi-v1.csv";
#endif
  } else { // MPIv2
#ifdef _OPENMP
    stats_path = "output/mpi-omp-v2.csv";
#else
    stats_path = "output/mpi-v2.csv";
#endif
  }

  // Creating folder; not checking errors as it can already exists
  mkdir("output", 0777);

  // Checking if the file already exists
  if (access(stats_path, W_OK)) {
    // File does not exists (or it cannot be written to);
    // creating new file with write permissions
    stats_fp = fopen(stats_path, "w+");
    if (stats_fp == NULL) {
      perror("Error creating output file");
      return -1;
    }
    // Write header
    fprintf(stats_fp, "m,n,k,processes,threads,error,");
    fprintf(stats_fp, "generation_time,first_communication_time,second_communication_time,parallel_time,serial_time\n");
  } else {
    // File exists; opening
    stats_fp = fopen(stats_path, "a");
    if (stats_fp == NULL) {
      perror("Error opening output file");
      return -1;
    }
  }

  fprintf(stats_fp, "%d,%d,%d,%d,%d,%f,", m, n, k, stats->processes, stats->threads, error);
  fprintf(stats_fp, "%f,%f,%f,%f,%f\n", stats->generation_time, stats->first_communication_time,
          stats->second_communication_time, stats->parallel_time, serial_time);

  fclose(stats_fp);
  return 0;
}
