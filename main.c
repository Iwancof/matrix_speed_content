#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

int inner_main();
int inner_main_block_test();
int inner_main_unit_block_test();

int main() {
  srand(0);

  // inner_main_unit_block_test();
  inner_main();
}

int inner_main_unit_block_test() {
  block *left, *right, *dest;
  INDEX_TYPE x, y;

  left = NULL;
  right = NULL;
  dest = NULL;

  left = allocate_block();
  right = allocate_block();
  dest = allocate_block();

  for_each_element(x, y) { left->element[y][x] = (double)rand() / RAND_MAX; }
  for_each_element(x, y) { right->element[y][x] = (double)rand() / RAND_MAX; }

  puts("left");
  show_block(left);
  puts("right");
  show_block(right);

  left_pre_block(right);
  BLOCK_MULT(left, right, dest);
  left_pre_block(dest);

  puts("dest");
  show_block(dest);

  return 0;
}

int inner_main_block_test() {
  block *blk1 = allocate_block();
  block *blk2 = allocate_block();
  block *dest = allocate_block();
  INDEX_TYPE bx, by;

  for_each_element(bx, by) {
    blk1->element[by][bx] = (double)rand() / RAND_MAX;
    blk2->element[by][bx] = (double)rand() / RAND_MAX;
  }

  // puts("blk1");
  // show_block(blk1);
  puts("blk2");
  show_block(blk2);

  left_pre_block(blk2);

  BLOCK_MULT(blk1, blk2, dest);

  return 0;
}

#if REDUCTION_TYPE == REDUCTION_DISABLE_INSAME
int inner_main() {
  matrix *left = map_matrix();
  matrix *right = map_matrix();

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

  // matrix *lefts[PARALLEL], *rights[PARALLEL], *dests[PARALLEL];
  matrix *lefts[PARALLEL], *rights[PARALLEL], *dests;
  for (int i = 0; i < PARALLEL; i++) {
    matrix* tmp;
    tmp = lefts[i] = map_matrix();
    matrix_copy(left, tmp);
    tmp = rights[i] = map_matrix();
    matrix_copy(right, tmp);
  }
  dests = map_matrix();

  double start, end;
  puts("allocated");

  start = omp_get_wtime();
  matrix_mult_per_block((const matrix* const restrict*)lefts, (const matrix* const restrict*)rights, dests);
  end = omp_get_wtime();

  printf("%f\n", end - start);

  return 0;
}

#else
int inner_main() {

  matrix *left = map_matrix();
  matrix *right = map_matrix();
  matrix *dest1 = map_matrix();
  // matrix *dest2 = allocate_matrix();

  matrix *thread_memo[PARALLEL];
  for (int i = 0; i < PARALLEL; i++) {
    // thread_memo[i] = map_matrix();
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

  left_pre_matrix(right); // not needed for benchmark

  // puts("[+] matrix allocated!");

  double start, end;

  start = omp_get_wtime();
  matrix_mult_per_block(left, right, dest1, thread_memo);
  end = omp_get_wtime();
  // printf("[+] elapsed %f\n", end - start);
  printf("%fGFLOPS\n", ((double)MATRIX_SIZE * (double)MATRIX_SIZE * (double)MATRIX_SIZE * 2 / 1024 / 1024 / 1024) / (end - start));
  // printf("%f\n", end - start);

  /*
  puts("vanilla");
  start = omp_get_wtime();
  matrix_mult_vanilla(left, right, dest2);
  end = omp_get_wtime();
  printf("elapsed %f\n", end - start);
  */

  unmap_matrix(left);
  unmap_matrix(right);
  unmap_matrix(dest1);

  for (int i = 0; i < PARALLEL; i++) {
    // unmap_matrix(thread_memo[i]);
  }

  return 0;
}
#endif
