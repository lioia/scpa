#ifndef UTILS_H
#define UTILS_H

// NOTE: maybe this should be split up based on responsability

#include <stdlib.h>

typedef enum { ZERO, INDEX, RANDOM } gen_type_t;

// Read a `.ini` file with the configuration
int read_configuration(char *config, int *m, int *n, int *k, gen_type_t *type, unsigned long *seed);

/*
 * Return a matrix of rows * cols size; values are based on `type`:
 *  - ZERO: empty matrix
 *  - INDEX: cell(i, j) has the value i * cols + j
 *  - RANDOM: every value is a random value in range [0, 1]
 * */
float *matrix_init(size_t rows, size_t cols, gen_type_t type);
// Print a matrix
void matrix_print(float *matrix, size_t rows, size_t cols);

// Serial implementation of C = C + A * B
void serial_mult(float *a, float *b, float *c, size_t m, size_t n, size_t k);

#endif // !UTILS_H
