#ifndef MPI_UTILS_H
#define MPI_UTILS_H

// Parse argument as a int (aborting on failure)
int parse_int_arg(char *arg);
// Calculate (start, end) based on the number of processes and dimension of the block
// offset has to be 0 for rows and (coords[0] * dims[0]) for cols
void calculate_start_end(int size, int dims, int coord, int offset, int *start, int *end);

#endif