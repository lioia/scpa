#include "utils.h"

// Calculate start and end based on coord
void calculate_start_end(int size, int dims, int coord, int offset, int *start, int *end) {
  int block_size = size / dims;
  int rest = size % dims; // Used to fairly distribute work
  *start = (offset + coord) * block_size % size;
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
