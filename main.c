#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"

int main() {
  matrix *mat = allocate_matrix();
  printf("%p\n", mat);

  puts("created");

  return 0;
}

