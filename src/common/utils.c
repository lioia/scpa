#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
  return sqrt(sum);
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

  return norm_diff / norm_c;
}

int write_stats(char *name, int m, int n, int k, int p, int t, double error, double generation_time,
                double parallel_time, double serial_time, double first_comm_time, double last_comm_time) {
  // Creating folder; not checking errors as it can already exists
  mkdir("output", 0777);
  // Calculating filepath size; + 8 is for `output`, `/` and `\0`
  int path_size = strlen(name) + 8;
  // Allocate memory for filename
  char *path = malloc(sizeof(*path) * path_size);
  if (path == NULL) {
    perror("Error allocating memory for filename");
    return -1;
  }
  // Write filename path
  sprintf(path, "output/%s", name);
  // NULL-terminate the string
  path[path_size - 1] = '\0';

  FILE *fp;
  // Checking if the file already exists
  if (access(path, W_OK)) {
    // File does not exists (or it cannot be written to);
    // creating new file with write permissions
    fp = fopen(path, "w+");
    if (fp == NULL) {
      perror("Error creating output file");
      return -1;
    }
    // Write header
    fprintf(fp, "m,n,k,p,t,error,g_t,p_t,s_t,comm_1_t,comm_2_t\n");
  } else {
    // File exists; opening
    fp = fopen(path, "a");
    if (fp == NULL) {
      perror("Error opening output file");
      return -1;
    }
  }
  // Write stats
  fprintf(fp, "%d,%d,%d,%d,%d,%f,", m, n, k, p, t, error);
  fprintf(fp, "%f,%f,%f,%f,%f\n", generation_time, parallel_time, serial_time, first_comm_time, last_comm_time);
  fclose(fp);
  free(path);
  return 0;
}
