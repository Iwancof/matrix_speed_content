#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

#include "matrix.h"

int main() {
  srand(0);

  matrix *left = allocate_matrix();
  matrix *right = allocate_matrix();
  matrix *dest1 = allocate_matrix();
  matrix *dest2 = allocate_matrix();

  matrix *thread_memo[PARALLEL];
  for(int i = 0;i < PARALLEL;i++) {
    thread_memo[i] = allocate_matrix();
  }

  INDEX_TYPE x, y, bx, by;
  for_each_blocks(x, y) {
    for_each_element(bx, by) {
      left->blocks[y][x].element[by][bx] = (double)rand() / RAND_MAX;
    }
  }
  for_each_blocks(x, y) {
    for_each_element(bx, by) {
      right->blocks[y][x].element[by][bx] = (double)rand() / RAND_MAX;
    }
  }

  puts("created");

  double start, end;

  puts("per block");
  start = omp_get_wtime();
  matrix_mult_per_block(left, right, dest1, thread_memo);
  end = omp_get_wtime();
  printf("elapsed %f\n", end - start);

  puts("vanilla");
  start = omp_get_wtime();
  matrix_mult_vanilla(left, right, dest2);
  end = omp_get_wtime();
  printf("elapsed %f\n", end - start);

  return 0;
}

