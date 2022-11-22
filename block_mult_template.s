.globl block_mult_asm
.text

block_mult_asm:
  @alias left = %rdi;
  @alias right = %rsi;
  @alias dest = %rdx;

  @alias left_fragment0 = %ymm0;
  @alias left_fragment1 = %ymm1;
  @alias left_fragment2 = %ymm2;
  @alias left_fragment3 = %ymm3;

  @alias right_fragment0 = %ymm4;
  @alias right_fragment1 = %ymm5;
  @alias right_fragment2 = %ymm6;
  @alias right_fragment3 = %ymm7;

  @alias sum0 = %ymm8;
  @alias sum1 = %ymm9;
  @alias sum2 = %ymm10;
  @alias sum3 = %ymm11;

  @builtin_func(LOAD_LEFT_FRAGMENT)(left_fragment, left, 0)
// @builtin_func(LOAD_RIGHT_FRAGMENT)(right_fragment, right, 

  ret
