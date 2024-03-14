#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "matrix.h"
#include "utils.h"

// Helper internal function to write to generic FILE*
int matrix_write_to_file(FILE *fp, float *matrix, int rows, int cols);

float *matrix_init(int rows, int cols) {
  float *matrix = malloc(sizeof(*matrix) * rows * cols);
  if (matrix == NULL) {
    perror("Error allocating matrix");
    return NULL;
  }
  for (size_t i = 0; i < rows * cols; i++)
    matrix[i] = i;
  // matrix[i] = (float)rand() / RAND_MAX;
  return matrix;
}

int matrix_write_bin_to_file(char *folder, char *name, float *matrix, int rows, int cols) {
  char *filename = create_file_path(folder, name);
  if (filename == NULL)
    return -1;
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror("Error writing to file (binary)");
    return -1;
  }
  fwrite(matrix, sizeof(*matrix), rows * cols, fp);
  fclose(fp);
  free(filename);
  return 0;
}

int matrix_write_txt_to_file(char *folder, char *name, float *matrix, int rows, int cols) {
  char *filename = create_file_path(folder, name);
  if (filename == NULL)
    return -1;
  FILE *fp = fopen(filename, "w+");
  if (fp == NULL) {
    perror("Error writing to file (text)");
    return -1;
  }
  matrix_write_to_file(fp, matrix, rows, cols);
  fclose(fp);
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
// NOTE: change order to see what is the fastest
void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k) {
  for (size_t i = 0; i < m; i++)
    for (size_t j = 0; j < n; j++)
      for (size_t l = 0; l < k; l++)
        c[i * n + j] += a[i * k + l] * b[l * n + j];
}
