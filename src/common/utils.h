#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define SEED 123456789

// Parse argument as a int (aborting on failure)
int parse_int_arg(char *arg);

// Calculation error
float calculate_error(float *c, float *c_serial, int m, int n);

// Concat file path
char *concat_path(char *path, char *name);

// Create stats file
FILE *open_stats_file(char *name);

// Get time using syscall
double get_time_syscall();

#endif // !UTILS_H
