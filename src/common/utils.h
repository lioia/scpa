#ifndef UTILS_H
#define UTILS_H

#define SEED 123456789

typedef struct {
  int processes;
  int threads;
  double first_communication_time;
  double second_communication_time;
  double generation_time;
  double parallel_time;
} stats_t;

typedef enum { MPIv1, MPIv2, OMP } version_t;

// Parse argument as a int (aborting on failure)
int parse_int_arg(char *arg);

// Calculation error
float calculate_error(float *c, float *c_serial, int m, int n);

// Get time using syscall
double get_time_syscall();

// Tasks to be done after computation has finished:
// - check with serial implementation
// - write stats to file
int root_tasks(float *a, float *b, float *c, float *c_serial, int m, int n, int k, stats_t *stats, version_t version,
               int iteration);

#endif // !UTILS_H
