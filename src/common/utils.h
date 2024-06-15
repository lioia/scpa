#ifndef UTILS_H
#define UTILS_H

// Parse argument as a int (aborting on failure)
int parse_int_arg(char *arg);

// Calculation error
float calculate_error(float *c, float *c_file, int m, int n);

// Write stats to file
int write_stats(char *name, int m, int n, int k, int p, int t, double error, double generation_time,
                double parallel_time, double serial_time, double first_comm_time, double last_comm_time);

#endif // !UTILS_H
