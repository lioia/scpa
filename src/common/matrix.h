#ifndef MATRIX_H
#define MATRIX_H

// Matrix helper functions

// Create a rows * cols matrix;
// type can be 0 (random) or 1 (index) and determines the value of the elements in the matrix
float *matrix_init(int rows, int cols, int type);
// Write matrix to filename (bin format)
int matrix_write_bin_to_file(char *folder, char *name, float *matrix, int rows, int cols);
// Write matrix to filename (txt format)
int matrix_write_txt_to_file(char *folder, char *name, float *matrix, int rows, int cols);
// Print matrix to stdout
void matrix_print(float *matrix, int rows, int cols);
// Serial implementation
void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k);
// Transpose matrix
void matrix_transpose(float *source, float *transposed, int rows, int cols);
// Parallel implementation
void matrix_parallel_mult(float *a, float *b, float *c, int m, int n, int k, int row_offset, int col_offset);

#endif // !MATRIX_H