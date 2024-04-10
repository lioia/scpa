#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "matrix.h"
#include "utils.h"

// Helper internal function to write to generic FILE*
int matrix_write_to_file(FILE *fp, float *matrix, int rows, int cols);

float *matrix_init(int rows, int cols, int type) {
  // Allocate memory needed
  float *matrix = malloc(sizeof(*matrix) * rows * cols);
  if (matrix == NULL) {
    perror("Error allocating matrix");
    return NULL;
  }
  // Set elements value (index or random, based on type)
  for (size_t i = 0; i < rows * cols; i++)
    matrix[i] = type ? i : (float)rand() / RAND_MAX;
  return matrix;
}

int matrix_write_bin_to_file(char *folder, char *name, float *matrix, int rows, int cols) {
  // Create output file path
  char *filename = create_file_path(folder, name);
  if (filename == NULL)
    return -1;
  // Open output file
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror("Error writing to file (binary)");
    return -1;
  }
  // Write as a blob of bytes
  fwrite(matrix, sizeof(*matrix), rows * cols, fp);
  // Close file
  fclose(fp);
  // Free memory used by the function
  free(filename);
  return 0;
}

int matrix_write_txt_to_file(char *folder, char *name, float *matrix, int rows, int cols) {
  // Create output file path
  char *filename = create_file_path(folder, name);
  if (filename == NULL)
    return -1;
  // Open output file
  FILE *fp = fopen(filename, "w+");
  if (fp == NULL) {
    perror("Error writing to file (text)");
    return -1;
  }
  // Write to file
  matrix_write_to_file(fp, matrix, rows, cols);
  // Close file
  fclose(fp);
  // Free memory used by the function
  free(filename);
  return 0;
}

void matrix_print(float *matrix, int rows, int cols) { matrix_write_to_file(stdout, matrix, rows, cols); }

int matrix_write_to_file(FILE *fp, float *matrix, int rows, int cols) {
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      fprintf(fp, "%f", matrix[i * cols + j]);
      if (j != cols - 1)
        fprintf(fp, ", ");
    }
    if (i != rows - 1)
      fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  return 0;
}

void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k) {
  for (size_t i = 0; i < m; i++) {
    for (size_t l = 0; l < k; l++) {
      float a_tmp = a[i * k + l];
      for (size_t j = 0; j < n; j++) {
        c[i * n + j] += a_tmp * b[l * n + j];
      }
    }
  }
}

void matrix_parallel_mult(float *a, float *b, float *c, int m, int n, int k, int offset) {
  int i, j, l;
  // Using collapse(2) instead of 3 to optimize write access
  // (with collapse 3, the tmp variable cannot be used)
#pragma omp parallel for private(i, j, l) shared(a, b, c) collapse(2)
  // Parallel Computation
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      float tmp = 0.0;
#pragma omp simd
      for (l = 0; l < k; l++) {
        tmp += a[i * k + l] * b[j * k + l];
      }
      c[i * n + j + offset] += tmp;
    }
  }
}
