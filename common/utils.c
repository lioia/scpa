#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "utils.h"

#define MAX_LINE_LENGTH 1024

int read_configuration(char *config, int *m, int *n, int *k, gen_type_t *type, unsigned long *seed) {
  // Opening configuration file
  FILE *fp = fopen(config, "r");
  if (fp == NULL) {
    perror("Error opening file (read config)");
    return -1;
  }
  // Initial seed value; set as current time in seconds or microseconds (if available)
  *seed = time(NULL);
  struct timeval now;
  if (!gettimeofday(&now, NULL))
    *seed = now.tv_sec * 1000000 + now.tv_usec;

  // Reading line by line
  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), fp) != NULL) {
    // Skipping comments or new-line
    if (line[0] == ';' || line[0] == '#' || line[0] == '\r' || line[0] == '\n')
      continue;
    // Splitting the line based on <key> = <value>
    char *save_ptr;
    char *key = strtok_r(line, "=", &save_ptr);
    char *value = strtok_r(NULL, "\n", &save_ptr);
    // No key or value in the split
    if (key == NULL || value == NULL)
      continue;
    // Parsing values
    if (!strncmp(key, "M", 1))
      *m = atoi(value);
    else if (!strncmp(key, "N", 1))
      *n = atoi(value);
    else if (!strncmp(key, "K", 1))
      *k = atoi(value);
    else if (!strncmp(key, "SEED", 4))
      *seed = strtol(value, NULL, 10);
    else if (!strncmp(key, "GEN", 3)) {
      // Trim whitespace
      while (isspace((unsigned char)*value))
        value += 1;
      // Parse gen type value
      if (!strncmp(value, "ZERO", 4))
        *type = ZERO;
      else if (!strncmp(value, "INDEX", 4))
        *type = INDEX;
      if (!strncmp(value, "RANDOM", 4))
        *type = RANDOM;
    } else
      fprintf(stderr, "Unrecognized tuple: %s, %s\n", key, value);
  }
  return 0;
}

float *matrix_init(size_t rows, size_t cols, gen_type_t type) {
  // Allocating the matrix as a contiguous array of size rows * cols
  float *matrix = malloc(sizeof(*matrix) * rows * cols);
  if (matrix == NULL) {
    perror("Error allocating matrix");
    return NULL;
  }
  memset(matrix, 0, sizeof(*matrix) * rows * cols);
  // Early return for base case of zero values in matrix
  if (type == ZERO)
    return matrix;

  // Filling matrix based on type
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      if (type == INDEX)
        matrix[i * cols + j] = i * cols + j;
      else
        matrix[i * cols + j] = i * cols + j;
    }
  }

  return matrix;
}

void matrix_print(float *matrix, size_t rows, size_t cols) {
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      printf("%6.6f", matrix[i * cols + j]);
      if (j != cols - 1)
        printf(", ");
    }
    if (i != rows - 1)
      printf("\n");
  }
  printf("\n");
}

// NOTE: change order to see what is the fastest
void serial_mult(float *a, float *b, float *c, size_t m, size_t n, size_t k) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int l = 0; l < k; l++) {
        c[i * n + j] += a[i * k + l] * b[l * n + j];
      }
    }
  }
}
