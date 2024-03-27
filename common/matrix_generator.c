#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "matrix.h"
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
  if (argc < 4) {
    printf("Usage: %s <m> <n> <k>[ <type>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Parsing arguments
  int m = strtol(argv[1], NULL, 10);
  int n = strtol(argv[2], NULL, 10);
  int k = strtol(argv[3], NULL, 10);
  // NOTE: with this implementation the first numbers of the matrices are always the same
  srand(SEED); // Currently set as a define; can be probably an argument
  // Initializing all the matrices
  int type = 0; // Random by default
  if (argc == 5 && !strncmp(argv[4], "index", 5))
    type = 1; // Index if specified
  float *a = matrix_init(m, k, type);
  float *b = matrix_init(k, n, type);
  float *c = malloc(sizeof(*c) * m * n); // Not using matrix_init as it has to be an empty matrix
  if (a == NULL || b == NULL || c == NULL)
    exit(EXIT_FAILURE);
  memset(c, 0, sizeof(*c) * m * n);

  // Calculate c matrix in a serial computation
  matrix_serial_mult(a, b, c, m, n, k);

  // Create folder path
  char *folder = create_folder_path(m, n, k);
  if (folder == NULL)
    exit(EXIT_FAILURE);
  // Same as mkdir -p folder bash command
  if (mkdir_p(folder))
    exit(EXIT_FAILURE);

  // Writing A and B matrix to bin file
  if (matrix_write_bin_to_file(folder, "a.bin", a, m, k))
    exit(EXIT_FAILURE);
  if (matrix_write_bin_to_file(folder, "b.bin", b, k, n))
    exit(EXIT_FAILURE);
  if (matrix_write_bin_to_file(folder, "c.bin", c, m, n))
    exit(EXIT_FAILURE);

#ifdef DEBUG
  // Writing A and B matrix to txt file (only in debug mode)
  if (matrix_write_txt_to_file(folder, "a.txt", a, m, k))
    exit(EXIT_FAILURE);
  if (matrix_write_txt_to_file(folder, "b.txt", b, k, n))
    exit(EXIT_FAILURE);
  if (matrix_write_txt_to_file(folder, "c.txt", c, m, n))
    exit(EXIT_FAILURE);
#endif

  // Freeing memory used
  free(folder);
  free(a);
  free(b);
  return 0;
}
