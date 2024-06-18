#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "matrix.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

float *matrix_init(int rows, int cols, enum gen_type_t type, int seed) {
  // Allocate matrix
  // aligned_alloc: memory aligned allocation to improve compiler vectorization
  // this is applied only if it's possible, otherwise use classic malloc
  float *matrix;
  if (((rows * cols) % 16) == 0)
    matrix = aligned_alloc(16, sizeof(*matrix) * rows * cols);
  else
    matrix = malloc(sizeof(*matrix) * rows * cols);
  if (matrix == NULL) {
    perror("Error allocating matrix");
    return NULL;
  }
  if (type == ZERO) {
    memset(matrix, 0, sizeof(*matrix) * rows * cols);
    return matrix;
  }

  srand(seed);
  // Set elements value (index or random, based on type)
  for (int i = 0; i < rows * cols; i++)
    matrix[i] = type == INDEX ? i : (float)rand() / RAND_MAX;

  return matrix;
}

void matrix_print(float *matrix, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++)
      // printf("%.3f ", matrix[i * cols + j]);
      printf("%d ", (int)matrix[i * cols + j]);
    printf("\n");
  }
}

void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k) {
  for (int i = 0; i < m; i++) {
    for (int l = 0; l < k; l++) {
      float a_tmp = a[i * k + l];
      for (int j = 0; j < n; j++)
        c[i * n + j] += a_tmp * b[l * n + j];
    }
  }
}

void matrix_transpose(float *restrict source, float *restrict transposed, int rows, int cols) {
  int i, j;
#pragma omp parallel for private(i, j) shared(source, transposed)
  for (int j = 0; j < cols; j++)
#pragma omp simd
    for (int i = 0; i < rows; i++)
      transposed[j * rows + i] = source[i * cols + j];
}

void matrix_parallel_mult(float *restrict a, float *restrict b, float *restrict c, int sub_m, int sub_n, int k, int n,
                          int row_offset, int col_offset) {
  int i, j, l, ii, jj, ll;
#pragma omp parallel for private(i, j, l) collapse(3)
  for (i = 0; i < sub_m; i += 16) {
    for (j = 0; j < sub_n; j += 16) {
      for (l = 0; l < k; l += 16) {
#pragma omp parallel for private(ii, jj, ll) shared(a, b, c) collapse(2)
        // Block multiplication
        for (ii = i; ii < MIN(i + 16, sub_m); ++ii) {
          for (jj = j; jj < MIN(j + 16, sub_n); ++jj) {
            float sum = 0;
#pragma omp simd
            for (ll = l; ll < MIN(l + 16, k); ll++)
              sum += a[ii * k + ll] * b[jj * k + ll];
            c[(ii + row_offset) * n + (jj + col_offset)] += sum;
          }
        }
      }
    }
  }
}
