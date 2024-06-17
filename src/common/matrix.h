#ifndef MATRIX_H
#define MATRIX_H

enum gen_type_t { INDEX, RANDOM, ZERO };

// Matrix helper functions

// Create a rows * cols matrix; type determines the values of the element
float *matrix_init(int rows, int cols, enum gen_type_t type, int seed);

// Print matrix to stdout
void matrix_print(float *matrix, int rows, int cols);

// Serial implementation
void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k);

// Transpose matrix
void matrix_transpose(float *source, float *transposed, int rows, int cols);

// Parallel implementation
// restrict: pointer is not aliased (not used in another context); allows the compiler to do vectorization
void matrix_parallel_mult(float *restrict a, float *restrict b, float *restrict c, int m, int sub_n, int k, int n,
                          int row_offset, int col_offset);

#endif // !MATRIX_H
