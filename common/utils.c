#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "utils.h"

float *matrix_init(size_t rows, size_t cols, gen_type_t type) {
  float *matrix = malloc(sizeof(*matrix) * rows * cols);
  if (matrix == NULL) {
    perror("Error allocating matrix");
    return NULL;
  }
  memset(matrix, 0, sizeof(*matrix) * rows * cols);
  // Early return for base case of zero values in matrix
  if (type == ZERO)
    return matrix;

  // Setting the seed as the current microsecond or time() if it fails
  unsigned int seed = time(NULL);
  struct timeval now;
  if (gettimeofday(&now, NULL) < 0)
    perror("Error in gettimeofday. Using time() as seed");
  else
    seed = now.tv_sec * 1000000 + now.tv_usec;
  srand(seed);

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

void matrix_print(float *matrix, size_t rows, size_t cols, const char *name) {
  printf("Matrix %s\n", name);
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      printf("%6.6f", matrix[i * cols + j]);
      if (j != cols - 1)
        printf(", ");
    }
    printf("\n");
  }
  printf("\n");
}
