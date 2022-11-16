#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"


int main() {
  normal_matrix mat = allocate_normal_matrix(10, 20);
  for(size_t x = 0;x < 10;x++) {
    for(size_t y = 0; y < 20;y++) {
      double* d = at_normal(mat, x, y);
      *d = 100. * x + y;
    }
  }
  show_normal_matrix(mat);
}

