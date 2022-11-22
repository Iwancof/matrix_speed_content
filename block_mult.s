%include "settings"

.intel_syntax noprefix
  .file "block_mult.s"
  .section .rodata

  .text
  .globl block_mult_asm
  .type block_mult_asm, @function

// block_mult_asm(left, rightm dest)

block_mult_asm:
  mov rax, $PARALLEL
  ret
