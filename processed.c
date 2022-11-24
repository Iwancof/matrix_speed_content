#include "matrix.h"
#include <x86intrin.h>

void BLOCK_MULT(const block *const restrict left,
                const block *const restrict right, block *const restrict dest) {

  __m256d left_fragment0;
  __m256d left_fragment1;
  __m256d left_fragment2;
  __m256d left_fragment3;

  __m256d right_fragment0;
  __m256d right_fragment1;
  __m256d right_fragment2;
  __m256d right_fragment3;

  __m256d sum0;
  __m256d sum1;
  __m256d sum2;
  __m256d sum3;

  left_fragment0 = _mm256_load_pd(&left->element[0][0]);
  left_fragment1 = _mm256_load_pd(&left->element[0][4]);
  left_fragment2 = _mm256_load_pd(&left->element[0][8]);
  left_fragment3 = _mm256_load_pd(&left->element[0][12]);

  for (int left_up_down_counter = 0; left_up_down_counter < 4;
       left_up_down_counter++) {
    int left_y = 0;

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

    sum0 = _mm256_mul_pd(left_fragment0, right_fragment0);
    sum1 = _mm256_mul_pd(left_fragment0, right_fragment1);
    sum2 = _mm256_mul_pd(left_fragment0, right_fragment2);
    sum3 = _mm256_mul_pd(left_fragment0, right_fragment3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

    sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

    sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][12]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][12]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][12]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][12]);

    sum0 = _mm256_fmadd_pd(left_fragment3, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment3, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment3, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment3, right_fragment3, sum3);

    dest->element[(left_up_down_counter * 4) + 0][left_y] +=
        sum0[0] + sum0[1] + sum0[2] + sum0[3];
    dest->element[(left_up_down_counter * 4) + 1][left_y] +=
        sum1[0] + sum1[1] + sum1[2] + sum1[3];
    dest->element[(left_up_down_counter * 4) + 2][left_y] +=
        sum2[0] + sum2[1] + sum2[2] + sum2[3];
    dest->element[(left_up_down_counter * 4) + 3][left_y] +=
        sum3[0] + sum3[1] + sum3[2] + sum3[3];

    left_y += 1;

    left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
    left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
    left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
    left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

    sum0 = _mm256_mul_pd(left_fragment3, right_fragment0);
    sum1 = _mm256_mul_pd(left_fragment3, right_fragment1);
    sum2 = _mm256_mul_pd(left_fragment3, right_fragment2);
    sum3 = _mm256_mul_pd(left_fragment3, right_fragment3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

    sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

    sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

    sum0 = _mm256_fmadd_pd(left_fragment0, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment0, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment0, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment0, right_fragment3, sum3);

    dest->element[(left_up_down_counter * 4) + 0][left_y] +=
        sum0[0] + sum0[1] + sum0[2] + sum0[3];
    dest->element[(left_up_down_counter * 4) + 1][left_y] +=
        sum1[0] + sum1[1] + sum1[2] + sum1[3];
    dest->element[(left_up_down_counter * 4) + 2][left_y] +=
        sum2[0] + sum2[1] + sum2[2] + sum2[3];
    dest->element[(left_up_down_counter * 4) + 3][left_y] +=
        sum3[0] + sum3[1] + sum3[2] + sum3[3];

    left_y += 1;

    do {
      left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
      left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
      left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
      left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

      sum0 = _mm256_mul_pd(left_fragment0, right_fragment0);
      sum1 = _mm256_mul_pd(left_fragment0, right_fragment1);
      sum2 = _mm256_mul_pd(left_fragment0, right_fragment2);
      sum3 = _mm256_mul_pd(left_fragment0, right_fragment3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

      sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

      sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][12]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][12]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][12]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][12]);

      sum0 = _mm256_fmadd_pd(left_fragment3, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment3, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment3, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment3, right_fragment3, sum3);

      dest->element[(left_up_down_counter * 4) + 0][left_y] +=
          sum0[0] + sum0[1] + sum0[2] + sum0[3];
      dest->element[(left_up_down_counter * 4) + 1][left_y] +=
          sum1[0] + sum1[1] + sum1[2] + sum1[3];
      dest->element[(left_up_down_counter * 4) + 2][left_y] +=
          sum2[0] + sum2[1] + sum2[2] + sum2[3];
      dest->element[(left_up_down_counter * 4) + 3][left_y] +=
          sum3[0] + sum3[1] + sum3[2] + sum3[3];

      left_y += 1;

      left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
      left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
      left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
      left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

      sum0 = _mm256_mul_pd(left_fragment3, right_fragment0);
      sum1 = _mm256_mul_pd(left_fragment3, right_fragment1);
      sum2 = _mm256_mul_pd(left_fragment3, right_fragment2);
      sum3 = _mm256_mul_pd(left_fragment3, right_fragment3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

      sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

      sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

      sum0 = _mm256_fmadd_pd(left_fragment0, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment0, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment0, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment0, right_fragment3, sum3);

      dest->element[(left_up_down_counter * 4) + 0][left_y] +=
          sum0[0] + sum0[1] + sum0[2] + sum0[3];
      dest->element[(left_up_down_counter * 4) + 1][left_y] +=
          sum1[0] + sum1[1] + sum1[2] + sum1[3];
      dest->element[(left_up_down_counter * 4) + 2][left_y] +=
          sum2[0] + sum2[1] + sum2[2] + sum2[3];
      dest->element[(left_up_down_counter * 4) + 3][left_y] +=
          sum3[0] + sum3[1] + sum3[2] + sum3[3];

      left_y += 1;
    } while (left_y < 16);

    left_up_down_counter += 1;
    left_y -= 1;

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

    sum0 = _mm256_mul_pd(left_fragment0, right_fragment0);
    sum1 = _mm256_mul_pd(left_fragment0, right_fragment1);
    sum2 = _mm256_mul_pd(left_fragment0, right_fragment2);
    sum3 = _mm256_mul_pd(left_fragment0, right_fragment3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

    sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

    sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][12]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][12]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][12]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][12]);

    sum0 = _mm256_fmadd_pd(left_fragment3, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment3, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment3, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment3, right_fragment3, sum3);

    dest->element[(left_up_down_counter * 4) + 0][left_y] +=
        sum0[0] + sum0[1] + sum0[2] + sum0[3];
    dest->element[(left_up_down_counter * 4) + 1][left_y] +=
        sum1[0] + sum1[1] + sum1[2] + sum1[3];
    dest->element[(left_up_down_counter * 4) + 2][left_y] +=
        sum2[0] + sum2[1] + sum2[2] + sum2[3];
    dest->element[(left_up_down_counter * 4) + 3][left_y] +=
        sum3[0] + sum3[1] + sum3[2] + sum3[3];

    left_y -= 1;

    left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
    left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
    left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
    left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

    sum0 = _mm256_mul_pd(left_fragment3, right_fragment0);
    sum1 = _mm256_mul_pd(left_fragment3, right_fragment1);
    sum2 = _mm256_mul_pd(left_fragment3, right_fragment2);
    sum3 = _mm256_mul_pd(left_fragment3, right_fragment3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

    sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

    sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

    right_fragment0 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
    right_fragment1 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
    right_fragment2 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
    right_fragment3 =
        _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

    sum0 = _mm256_fmadd_pd(left_fragment0, right_fragment0, sum0);
    sum1 = _mm256_fmadd_pd(left_fragment0, right_fragment1, sum1);
    sum2 = _mm256_fmadd_pd(left_fragment0, right_fragment2, sum2);
    sum3 = _mm256_fmadd_pd(left_fragment0, right_fragment3, sum3);

    dest->element[(left_up_down_counter * 4) + 0][left_y] +=
        sum0[0] + sum0[1] + sum0[2] + sum0[3];
    dest->element[(left_up_down_counter * 4) + 1][left_y] +=
        sum1[0] + sum1[1] + sum1[2] + sum1[3];
    dest->element[(left_up_down_counter * 4) + 2][left_y] +=
        sum2[0] + sum2[1] + sum2[2] + sum2[3];
    dest->element[(left_up_down_counter * 4) + 3][left_y] +=
        sum3[0] + sum3[1] + sum3[2] + sum3[3];

    do {
      left_y -= 1;

      left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
      left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
      left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
      left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

      sum0 = _mm256_mul_pd(left_fragment0, right_fragment0);
      sum1 = _mm256_mul_pd(left_fragment0, right_fragment1);
      sum2 = _mm256_mul_pd(left_fragment0, right_fragment2);
      sum3 = _mm256_mul_pd(left_fragment0, right_fragment3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

      sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

      sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][12]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][12]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][12]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][12]);

      sum0 = _mm256_fmadd_pd(left_fragment3, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment3, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment3, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment3, right_fragment3, sum3);

      dest->element[(left_up_down_counter * 4) + 0][left_y] +=
          sum0[0] + sum0[1] + sum0[2] + sum0[3];
      dest->element[(left_up_down_counter * 4) + 1][left_y] +=
          sum1[0] + sum1[1] + sum1[2] + sum1[3];
      dest->element[(left_up_down_counter * 4) + 2][left_y] +=
          sum2[0] + sum2[1] + sum2[2] + sum2[3];
      dest->element[(left_up_down_counter * 4) + 3][left_y] +=
          sum3[0] + sum3[1] + sum3[2] + sum3[3];

      left_y -= 1;

      left_fragment0 = _mm256_load_pd(&left->element[left_y][0]);
      left_fragment1 = _mm256_load_pd(&left->element[left_y][4]);
      left_fragment2 = _mm256_load_pd(&left->element[left_y][8]);
      left_fragment3 = _mm256_load_pd(&left->element[left_y][12]);

      sum0 = _mm256_mul_pd(left_fragment3, right_fragment0);
      sum1 = _mm256_mul_pd(left_fragment3, right_fragment1);
      sum2 = _mm256_mul_pd(left_fragment3, right_fragment2);
      sum3 = _mm256_mul_pd(left_fragment3, right_fragment3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][8]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][8]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][8]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][8]);

      sum0 = _mm256_fmadd_pd(left_fragment2, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment2, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment2, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment2, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][4]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][4]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][4]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][4]);

      sum0 = _mm256_fmadd_pd(left_fragment1, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment1, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment1, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment1, right_fragment3, sum3);

      right_fragment0 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 0][0]);
      right_fragment1 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 1][0]);
      right_fragment2 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 2][0]);
      right_fragment3 =
          _mm256_load_pd(&right->element[(4 * left_up_down_counter) + 3][0]);

      sum0 = _mm256_fmadd_pd(left_fragment0, right_fragment0, sum0);
      sum1 = _mm256_fmadd_pd(left_fragment0, right_fragment1, sum1);
      sum2 = _mm256_fmadd_pd(left_fragment0, right_fragment2, sum2);
      sum3 = _mm256_fmadd_pd(left_fragment0, right_fragment3, sum3);

      dest->element[(left_up_down_counter * 4) + 0][left_y] +=
          sum0[0] + sum0[1] + sum0[2] + sum0[3];
      dest->element[(left_up_down_counter * 4) + 1][left_y] +=
          sum1[0] + sum1[1] + sum1[2] + sum1[3];
      dest->element[(left_up_down_counter * 4) + 2][left_y] +=
          sum2[0] + sum2[1] + sum2[2] + sum2[3];
      dest->element[(left_up_down_counter * 4) + 3][left_y] +=
          sum3[0] + sum3[1] + sum3[2] + sum3[3];
    } while (0 < left_y);
  }
}
