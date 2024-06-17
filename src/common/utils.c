#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

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

char *concat_path(char *path, char *name) {
  // Calculating filepath size; + 2 is for `/` and `\0`
  int path_size = strlen(path) + strlen(name) + 2;
  // Allocate memory for filename
  char *new_path = malloc(sizeof(*new_path) * path_size);
  if (new_path == NULL) {
    perror("Error allocating memory for filename");
    return NULL;
  }
  // Write filename path
  sprintf(new_path, "%s/%s", path, name);
  // NULL-terminate the string
  new_path[path_size - 1] = '\0';
  return new_path;
}

FILE *open_stats_file(char *name) {
  // Creating folder; not checking errors as it can already exists
  mkdir("output", 0777);
  char *path = concat_path("output", name);
  if (path == NULL)
    return NULL;

  FILE *fp;
  // Checking if the file already exists
  if (access(path, W_OK)) {
    // File does not exists (or it cannot be written to);
    // creating new file with write permissions
    fp = fopen(path, "w+");
    if (fp == NULL) {
      perror("Error creating output file");
      return NULL;
    }
    // Write header
    fprintf(fp, "m,n,k,p,t,g_t,p_t,comm_1_t,comm_2_t,s_t,error\n");
  } else {
    // File exists; opening
    fp = fopen(path, "a");
    if (fp == NULL) {
      perror("Error opening output file");
      return NULL;
    }
  }
  free(path);
  return fp;
}

double get_time_syscall() {
  // Get time with a system-call
  struct timeval tv;
  gettimeofday(&tv, NULL);
  // Convert to the same format used by omp_get_wtime and MPI_Wtime
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}
