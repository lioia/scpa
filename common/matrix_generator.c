#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"

#define SEED 12345

// mkdir recursively (like the `mkdir -p` command)
int mkdir_p(char *folder) {
  // strtok modifies the pointer
  char *dup_path = strdup(folder);
  // Splitting path by `/`
  char *token = strtok(dup_path, "/");
  char *dir = NULL;
  char current_dir[256] = ".";
  while (token != NULL) {
    strcat(current_dir, "/");
    strcat(current_dir, token); // Next subfolder
    // Creating folder
    if (mkdir(current_dir, 0777) != 0) {
      // Checking if the folder already exists
      struct stat st;
      if (stat(current_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error creating directory %s\n", current_dir);
        free(dup_path);
        return -1;
      }
    }
    // Preparing for next iteration
    token = strtok(NULL, "/");
  }
  free(dup_path);
  return 0;
}

int main(int argc, char **argv) {
  // Checking arguments
  if (argc != 4) {
    printf("Usage: %s <m> <n> <k>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Parsing arguments
  int m = strtol(argv[1], NULL, 10);
  int n = strtol(argv[2], NULL, 10);
  int k = strtol(argv[3], NULL, 10);
  // NOTE: with this implementation the first numbers of the matrices are always the same;
  // so probably something can be done based on the m, n, k parameters
  srand(SEED); // Currently set as a define; can be probably an argument
  // Initializing all the matrices
  float *a = matrix_init(m, k);
  float *b = matrix_init(k, n);
  float *c = malloc(sizeof(*c) * m * n); // Not using matrix_init as it has to be an empty matrix
  if (a == NULL || b == NULL || c == NULL)
    exit(EXIT_FAILURE);
  memset(c, 0, sizeof(*c) * m * n);

  matrix_serial_mult(a, b, c, m, n, k);
  // Calculating folder path size (+1 is for NULL-terminator)
  int folder_path_size = snprintf(NULL, 0, "matrix/%dx%dx%d/", m, n, k) + 1;
  // Allocating memory for folder string
  char *folder = malloc(sizeof(*folder) * folder_path_size);
  if (folder == NULL) {
    perror("Error allocating memory for folder path");
    exit(EXIT_FAILURE);
  }
  // Writing folder path
  snprintf(folder, folder_path_size * sizeof(*folder), "matrix/%dx%dx%d", m, n, k);
  folder[folder_path_size - 1] = '\0'; // NULL-terminated string
  // Same as mkdir -p folder bash command
  if (mkdir_p(folder))
    exit(EXIT_FAILURE);

  // Writing A and B matrix to bin file
  if (matrix_write_bin_to_file(folder, "a.bin", a, m, k))
    exit(EXIT_FAILURE);
  if (matrix_write_bin_to_file(folder, "b.bin", b, m, k))
    exit(EXIT_FAILURE);
  if (matrix_write_bin_to_file(folder, "c.bin", c, m, n))
    exit(EXIT_FAILURE);

  // NOTE: this part can probably be removed as it is redundant

  // Writing A and B matrix to txt file
  if (matrix_write_txt_to_file(folder, "a.txt", a, m, k))
    exit(EXIT_FAILURE);
  if (matrix_write_txt_to_file(folder, "b.txt", b, k, n))
    exit(EXIT_FAILURE);
  if (matrix_write_txt_to_file(folder, "c.txt", c, m, n))
    exit(EXIT_FAILURE);

  // Freeing memory used
  free(folder);
  free(a);
  free(b);
  return 0;
}
