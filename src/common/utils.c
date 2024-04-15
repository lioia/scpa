#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

// Return parsed int from arg; exits otherwise
int parse_int_arg(char *arg) {
  // Parse to an int
  int val = strtol(arg, NULL, 10);
  // Check errors
  if ((val == INT_MAX || val == INT_MIN) && errno == ERANGE) {
    fprintf(stderr, "Error parsing m (%s): out of range", arg);
    exit(EXIT_FAILURE);
  }
  return val;
}

char *create_folder_path(int m, int n, int k) {
  // Calculating folder path size (+1 is for NULL-terminator)
  int folder_path_size = snprintf(NULL, 0, "output/%dx%dx%d/", m, n, k) + 1;
  // Allocating memory for folder string
  char *folder = malloc(sizeof(*folder) * folder_path_size);
  if (folder == NULL) {
    perror("Error allocating memory for folder path");
    return NULL;
  }
  // Writing folder path
  snprintf(folder, folder_path_size * sizeof(*folder), "output/%dx%dx%d", m, n, k);
  folder[folder_path_size - 1] = '\0'; // NULL-terminated string
  return folder;
}

char *create_file_path(char *folder, char *name) {
  // Calculating filepath size; + 2 is for `/` and `\0`
  int filename_size = strlen(folder) + strlen(name) + 2;
  // Allocate memory for filename
  char *filename = malloc(sizeof(*filename) * filename_size);
  if (filename == NULL) {
    perror("Error allocating memory for filename");
    return NULL;
  }
  // Write filename path
  sprintf(filename, "%s/%s", folder, name);
  // NULL-terminate the string
  filename[filename_size - 1] = '\0';
  return filename;
}

float calculate_error(float *c, float *c_file, int m, int n) {
  float error = 0.0;
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      // NOTE: maybe use a better metric
      float diff = c[i * n + j] - c_file[i * n + j];
      error += diff > 0 ? diff : -diff;
    }
  }
  return error;
}

int write_stats(char *name, int m, int n, int k, int p, int t, float time, float error) {
  // Creating folder; not checking errors as it can already exists
  mkdir("output", 0777);
  char *path = create_file_path("output", name);
  if (path == NULL)
    return -1;
  FILE *fp;
  if (access(path, W_OK)) {
    // File does not exists (or it cannot be written to); creating new file with write permissions
    fp = fopen(path, "w+");
    if (fp == NULL) {
      perror("Error creating output file");
      return -1;
    }
    // Write header
    fprintf(fp, "m,n,k,p,t,time,error\n");
  } else {
    // File exists; opening
    fp = fopen(path, "a");
    if (fp == NULL) {
      perror("Error opening output file");
      return -1;
    }
  }
  // Write stats
  fprintf(fp, "%d,%d,%d,%d,%d,%f,%f\n", m, n, k, p, t, time, error);
  fclose(fp);
  free(path);
  return 0;
}
