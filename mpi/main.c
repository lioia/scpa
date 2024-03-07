#include <stdio.h>

#include "../common/utils.h"

int main(int argc, char **argv) {
  // Variable Declaration
  int m, n, k;
  unsigned long seed;
  float *a, *b, *c;
  gen_type_t type;
  // Variable Initialization
  if (read_configuration("configuration.ini", &m, &n, &k, &type, &seed))
    return -1;
  srand(seed);
  printf("Configuration: %d %d %d %d\n", m, n, k, type);
  a = matrix_init(m, k, type);
  b = matrix_init(k, n, type);
  c = matrix_init(m, n, ZERO);
  if (a == NULL || b == NULL || c == NULL)
    return -1;
  serial_mult(a, b, c, m, n, k);
  puts("A Matrix");
  matrix_print(a, m, k);
  puts("B Matrix");
  matrix_print(b, k, n);
  puts("C Matrix");
  matrix_print(c, m, n);
  return 0;
}
