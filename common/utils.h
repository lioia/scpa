#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

typedef enum { ZERO, INDEX, RANDOM } gen_type_t;

float *matrix_init(size_t rows, size_t cols, gen_type_t type);
void matrix_print(float *matrix, size_t rows, size_t cols, const char *name);

#endif // !UTILS_H
