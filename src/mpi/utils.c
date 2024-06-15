#include "utils.h"

void calculate_start_end(int size, int dims, int coord, int *start, int *end) {
  int block_size = size / dims;
  int rest = size % dims; // Used to fairly distribute work
  *start = coord * block_size % size;
  *end = 0;
  // Fairly distribute work based on the rank (coord in the topology)
  if (coord < rest) {
    *start += coord;
    *end += 1;
  } else {
    *start += rest;
  }
  *end += *start + block_size;
}
