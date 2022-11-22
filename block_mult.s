.globl block_mult_asm
.text

block_mult_asm:




  vmovapd 32(%rdi), %ymm0
  vmovapd 40(%rdi), %ymm1
  vmovapd 48(%rdi), %ymm2
  vmovapd 56(%rdi), %ymm3

  ret



