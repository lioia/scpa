#ifndef UTILS_H
#define UTILS_H

// Parse argument as a int (aborting on failure)
int parse_int_arg(char *arg);
// Create folder path based on m, n, k
char *create_folder_path(int m, int n, int k);
// Create file path for the matrix in file
char *create_file_path(char *folder, char *file);
// Calculation error
float calculate_error(float *c, float *c_file, int m, int n);

#endif // !UTILS_H
