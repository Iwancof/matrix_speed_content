#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"

int main() {
  matrix *mat = allocate_matrix();

  for_each_blocks(mat, this, {
      this->element[0][1] = 0.1;
      printf("%p\n", this);
  });

  show_matrix(mat);

  return 0;
}

