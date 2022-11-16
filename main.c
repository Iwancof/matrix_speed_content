#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"

int main() {
  matrix mat = allocate_matrix(0x20, 0x20);

  puts("created");

  for_each_blocks(mat, block, {
    printf("%p\n", block);
  });

  return 0;
}

