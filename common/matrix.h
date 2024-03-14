#ifndef MATRIX_H
#define MATRIX_H

// Matrix helper functions
// Creates a rows * cols matrix
float *matrix_init(int rows, int cols);
// Write matrix to filename (bin format)
int matrix_write_bin_to_file(char *folder, char *name, float *matrix, int rows, int cols);
// Write matrix to filename (txt format)
int matrix_write_txt_to_file(char *folder, char *name, float *matrix, int rows, int cols);
// Print matrix to stdout
void matrix_print(float *matrix, int rows, int cols);
// Serial implementation of C = C + A * B
void matrix_serial_mult(float *a, float *b, float *c, int m, int n, int k);

#endif // !MATRIX_H
