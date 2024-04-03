#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

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
  if (argc == 5 && !strncmp(argv[4], "index", 5) && !strncmp(argv[4], "random", 6)) {
    printf("Incorrect type parameter: expected index or random, got %s\n", argv[4]);
    exit(EXIT_FAILURE);
  }

  // Variable declaration
  int m, n, k;                 // Matrix dimension
  int type;                    // Generation type
  float *a, *b, *c;            // Matrices
  double start_time, end_time; // Computation start and end time
  char *folder;                // Output folder

  // Variable initialization
  type = 0;
  if (argc == 5)
    type = !strncmp(argv[4], "index", 5);
  m = parse_int_arg(argv[1]);
  n = parse_int_arg(argv[2]);
  k = parse_int_arg(argv[3]);
  start_time = 0.0;
  end_time = 0.0;

  // NOTE: with this implementation the first numbers of the matrices are always the same
  // SEED is currently a define; can probably be set as an additional argument
  srand(SEED);
  // Initializing all the matrices
  a = matrix_init(m, k, type);
  b = matrix_init(k, n, type);
  c = malloc(sizeof(*c) * m * n); // Not using matrix_init as it has to be an empty matrix
  if (a == NULL || b == NULL || c == NULL)
    exit(EXIT_FAILURE);
  memset(c, 0, sizeof(*c) * m * n);

  // Setting start time
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start_time = tv.tv_sec + tv.tv_usec / 1000000.0;

  // Calculate c matrix in a serial computation
  matrix_serial_mult(a, b, c, m, n, k);
  gettimeofday(&tv, NULL);
  // Setting end time
  end_time = tv.tv_sec + tv.tv_usec / 1000000.0;
  // Writing stats
  if (write_stats("serial.csv", m, n, k, 1, end_time - start_time, 0))
    exit(EXIT_FAILURE);

  // Create folder path
  folder = create_folder_path(m, n, k);
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
  free(c);
  return 0;
}
