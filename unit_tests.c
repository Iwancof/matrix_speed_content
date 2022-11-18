#include<CUnit/Console.h>
#include<CUnit/CUnit.h>
#include<CUnit/TestDB.h>

#include<stdlib.h>

#include "matrix.h"

void block_mult_unit_test();
void block_mult_random_test();
void matrix_mult_random_test();

int main() {
  CU_pSuite suite;
  CU_initialize_registry();

  suite = CU_add_suite("Block matrix test", NULL, NULL);

  CU_add_test(suite, "Block mult unit test", block_mult_unit_test);
  CU_add_test(suite, "Block mult random test", block_mult_random_test);
  CU_add_test(suite, "Matrix mult random test", matrix_mult_random_test);

  CU_console_run_tests();
  CU_cleanup_registry();

  return 0;
}

void block_mult_unit_test() {
  block *left, *right, *dest;
  INDEX_TYPE x, y;

  left = NULL;
  right = NULL;
  dest = NULL;

  posix_memalign((void**)&left, BLOCK_SIZE * BLOCK_SIZE, sizeof(block));
  posix_memalign((void**)&right, BLOCK_SIZE * BLOCK_SIZE, sizeof(block));
  posix_memalign((void**)&dest, BLOCK_SIZE * BLOCK_SIZE, sizeof(block));

  for_each_element(x, y) {
    if(x == y) {
      left->element[y][x] = 1.;
    } else {
      left->element[y][x] = 0.;
    }
  }

  for_each_element(x, y) {
    if(x == y) {
      right->element[y][x] = 1.;
    } else {
      right->element[y][x] = 0.;
    }
  }

  for_each_element(x, y) {
    dest->element[y][x] = 0;
  }

  BLOCK_MULT(left, right, dest);

  for_each_element(x, y) {
    if(x == y) {
      CU_ASSERT_DOUBLE_EQUAL(dest->element[y][x], 1., 0.01);
    } else {
      CU_ASSERT_DOUBLE_EQUAL(dest->element[y][x], 0., 0.01);
    }
  }

  BLOCK_MULT(left, right, dest);

  for_each_element(x, y) {
    if(x == y) {
      CU_ASSERT_DOUBLE_EQUAL(dest->element[y][x], 2., 0.01);
    } else {
      CU_ASSERT_DOUBLE_EQUAL(dest->element[y][x], 0., 0.01);
    }
  }
}

void block_mult_random_test() {
  FILE *f;
  char *content_ptr, *token, *context;
  size_t file_size, read, line_index;
  INNER_TYPE value_buf;
  INDEX_TYPE i, x, y;
  block *blocks[4];

  for(i = 0;i < 4;i++) {
    posix_memalign((void**)&blocks[i], BLOCK_SIZE * BLOCK_SIZE, sizeof(block));
  }

  f = fopen("block_test_value", "r");
  CU_ASSERT_PTR_NOT_NULL(f);

  fseek(f, 0, SEEK_END);
  file_size = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  content_ptr = (char*)malloc(file_size);

  read = fread(content_ptr, sizeof(char), file_size, f);
  CU_ASSERT_EQUAL(read, file_size);

  line_index = 0;
  token = strtok_r(content_ptr, ",", &context);
  while(token != NULL) {
    value_buf = atof(token);
    blocks[line_index / (BLOCK_SIZE * BLOCK_SIZE)]->element[(line_index % (BLOCK_SIZE * BLOCK_SIZE)) / BLOCK_SIZE][(line_index % (BLOCK_SIZE * BLOCK_SIZE)) % BLOCK_SIZE] = value_buf;

    line_index++;
    token = strtok_r(NULL, ",", &context);
  }

  BLOCK_MULT(blocks[0], blocks[1], blocks[3]);

  for_each_element(x, y) {
    CU_ASSERT_DOUBLE_EQUAL(blocks[2]->element[y][x], blocks[3]->element[y][x], 0.01);
  }
}

void matrix_mult_random_test() {
  FILE *f = fopen("matrix_test_value", "r");
  CU_ASSERT_PTR_NOT_NULL(f);

  fseek(f, 0, SEEK_END);
  size_t file_size = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);
  
  char* content_ptr = (char*)malloc(file_size * sizeof(char));
  CU_ASSERT_PTR_NOT_NULL(content_ptr);

  size_t read = fread(content_ptr, sizeof(char), file_size, f);
  CU_ASSERT_EQUAL(file_size, read);

  size_t line_index = 0;
  char *context, *token;

  matrix *thread_memo[PARALLEL];
  for(int i = 0;i < PARALLEL;i++) {
    thread_memo[i] = allocate_matrix();
  }

  matrix *matries[4];
  for(int i = 0;i < 4;i++) {
    matries[i] = allocate_matrix();
  }

  token = strtok_r(content_ptr, ",", &context);
  while(token != NULL) {
    double value_buf = atof(token);

    size_t matrix_index = line_index / (MATRIX_SIZE * MATRIX_SIZE);
    size_t matrix_offset = line_index % (MATRIX_SIZE * MATRIX_SIZE);
    size_t block_y = matrix_offset / MATRIX_SIZE / BLOCK_SIZE;
    size_t block_x = matrix_offset % MATRIX_SIZE / BLOCK_SIZE;
    size_t in_block_y = matrix_offset / MATRIX_SIZE % BLOCK_SIZE;
    size_t in_block_x = matrix_offset % BLOCK_SIZE;

    matries[matrix_index]->blocks[block_y][block_x].element[in_block_y][in_block_x] = value_buf;

    line_index += 1;
    token = strtok_r(NULL, ",", &context);
  }

  matrix_mult_per_block(matries[0], matries[1], matries[3], thread_memo);

  size_t block_x, block_y, elm_x, elm_y;
  for_each_blocks(block_x, block_y) {
    for_each_element(elm_x, elm_y) {
      CU_ASSERT_DOUBLE_EQUAL(
          matries[2]->blocks[block_y][block_x].element[elm_y][elm_x],
          matries[3]->blocks[block_y][block_x].element[elm_y][elm_x],
          0.01
        );
    }
  }
}
