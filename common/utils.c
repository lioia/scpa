#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Return parsed int from arg; exits otherwise
int parse_int_arg(char *arg) {
  int val = strtol(arg, NULL, 10);
  if ((val == INT_MAX || val == INT_MIN) && errno == ERANGE) {
    fprintf(stderr, "Error parsing m (%s): out of range", arg);
    exit(EXIT_FAILURE);
  }
  return val;
}

char *create_folder_path(int m, int n, int k) {
  // Calculating folder path size (+1 is for NULL-terminator)
  int folder_path_size = snprintf(NULL, 0, "matrix/%dx%dx%d/", m, n, k) + 1;
  // Allocating memory for folder string
  char *folder = malloc(sizeof(*folder) * folder_path_size);
  if (folder == NULL) {
    perror("Error allocating memory for folder path");
    return NULL;
  }
  // Writing folder path
  snprintf(folder, folder_path_size * sizeof(*folder), "matrix/%dx%dx%d", m, n, k);
  folder[folder_path_size - 1] = '\0'; // NULL-terminated string
  return folder;
}

char *create_file_path(char *folder, char *name) {
  int filename_size = strlen(folder) + strlen(name) + 2;
  char *filename = malloc(sizeof(*filename) * filename_size);
  sprintf(filename, "%s/%s", folder, name);
  filename[filename_size - 1] = '\0';
  return filename;
}
