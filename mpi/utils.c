#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Return parsed int from arg; exits otherwise
int parse_int_arg(char *arg) {
  int val = strtol(arg, NULL, 10);
  if ((val == INT_MAX || val == INT_MIN) && errno == ERANGE) {
    fprintf(stderr, "Error parsing m (%s): out of range", arg);
    exit(EXIT_FAILURE);
  }
  return val;
}

// Calculate start and end based on coord
void calculate_start_end(int size, int dims, int coord, int offset, int *start, int *end) {
  int block_size = size / dims;
  int rest = size % dims; // Used to fairly distribute work
  *start = (offset + coord) * block_size % size;
  *end = 0;
  if (coord < rest) {
    *start += coord;
    *end += 1;
  } else {
    *start += rest;
  }
  *end += *start + block_size;
}
