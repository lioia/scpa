#ifdef _OPENMP
#include <omp.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "../common/matrix.h"
#include "../common/utils.h"

int main(int argc, char **argv) {
  // TODO: check on argc; print usage
  // TODO: errno range check (copy from mpi)
  int m = strtol(argv[1], NULL, 10);
  int n = strtol(argv[2], NULL, 10);
  int k = strtol(argv[3], NULL, 10);
  int p = strtol(argv[4], NULL, 10);
  int i, j, l;
  double start_time = 0.0;
  double end_time = 0.0;
#ifdef _OPENMP
  omp_set_num_threads(p);
  start_time = omp_get_wtime();
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start_time = tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
  char *folder = create_folder_path(m, n, k);
  if (folder == NULL)
    return -1;
  char *a_path = create_file_path(folder, "a.bin");
  char *b_path = create_file_path(folder, "b.bin");
  char *c_path = create_file_path(folder, "c.bin");
  if (a_path == NULL || b_path == NULL || c_path == NULL)
    return -1;
  FILE *a_fp = fopen(a_path, "r");
  FILE *b_fp = fopen(b_path, "r");
  FILE *c_fp = fopen(c_path, "r");
  if (a_fp == NULL || b_fp == NULL || c_fp == NULL) {
    perror("File");
    return -1;
  }
  float *a = malloc(sizeof(*a) * m * k);
  float *b = malloc(sizeof(*b) * n * k);
  float *c = malloc(sizeof(*c) * m * m);
  float *c_file = malloc(sizeof(*c_file) * m * m);
  if (a == NULL || b == NULL || c == NULL || c_file == NULL) {
    perror("Alloc");
    return -1;
  }
  fread(a, sizeof(*a), m * k, a_fp);
  fread(b, sizeof(*b), n * k, b_fp);
  fread(c_file, sizeof(*c_file), n * m, c_fp);
#pragma omp parallel for private(i, j, l) shared(a, b, c) collapse(2)
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      for (l = 0; l < k; l++)
        c[i * n + j] += a[i * k + l] * b[l * n + j];
  float error = 0.0;
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < m; j++) {
      float diff = c[i * n + j] - c_file[i * n + j];
      error += diff > 0 ? diff : -diff;
    }
  }
#ifdef _OPENMP
  end_time = omp_get_wtime();
#else
  gettimeofday(&tv, NULL);
  end_time = tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
  printf("m,n,k,p,time,error\n");
  printf("%d,%d,%d,%d,%f,%f\n", m, n, k, p, end_time - start_time, error);
  matrix_print(c, m, n);
  return 0;
}
