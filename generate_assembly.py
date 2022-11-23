load_counter = 0
mult_counter = 0
fmadd_counter = 0
fused_counter = 0
writeback_counter = 0

asm = '''
.text
.globl block_mult_asm

block_mult_asm:
'''

def left_index(x, y):
    return (y * 16 + x) * 8

def right_index(x, y):
    return (x * 16 + y) * 8

def vmovapd(src = None, dest = None):
    global load_counter, asm

    assert src is not None
    assert dest is not None

    load_counter += 1
    asm += f'vmovapd {src}, {dest}\n'

def mul(src1 = None, src2 = None, dest = None):
    global mult_counter, asm

    assert src1 is not None
    assert src2 is not None
    assert dest is not None

    mult_counter += 1
    asm += f'vmulpd {src1}, {src2}, {dest}\n'

def fmadd(src1 = None, src2 = None, src3_dest = None):
    global fmadd_counter, asm

    assert src1 is not None
    assert src2 is not None
    assert dest is not None

    fmadd_counter += 1
    asm += f'vfmadd231pd {src1}, {src2}, {src3_dest}\n'

    # src3_dest += src1 * src2

"""
#define LOAD_LEFT_FRAGMENT(frag, left, y)                                      \
  do {                                                                         \
    frag##0 = _mm256_load_pd(&left->element[y][0]);                            \
    frag##1 = _mm256_load_pd(&left->element[y][4]);                            \
    frag##2 = _mm256_load_pd(&left->element[y][8]);                            \
    frag##3 = _mm256_load_pd(&left->element[y][12]);                           \
  } while (0)
"""

prev_left_load = None
def LOAD_LEFT_FRAGMENT(frag, left, y):
    global load_counter, prev_left_load, asm

    if prev_left_load == (left, y):
        print(f'found extra load! left')
        quit(0)
    prev_left_load = (left, y)

    asm += '\n'
    for i in range(4):
        vmovapd(src = f'{left_index(i * 4, y)}({left})',
               dest = f'{frag[i]}')

"""
#define LOAD_RIGHT_FRAGMENT(frag, right, base_x, y)                            \
  do {                                                                         \
    frag##0 = _mm256_load_pd(&right->element[(base_x) + 0][y]);                \
    frag##1 = _mm256_load_pd(&right->element[(base_x) + 1][y]);                \
    frag##2 = _mm256_load_pd(&right->element[(base_x) + 2][y]);                \
    frag##3 = _mm256_load_pd(&right->element[(base_x) + 3][y]);                \
  } while (0)
"""

prev_load_right = None
def LOAD_RIGHT_FRAGMENT(frag, right, base_x, y):
    global prev_load_right, asm

    if prev_load_right == (base_x, y):
        print('found extra load! right')
        quit(0)
    prev_load_right = (base_x, y)

    asm += '\n'
    for i in range(4):
        vmovapd(src = f'{right_index(base_x + i, y)}({right})', 
               dest = f'{frag[i]}')

"""
#define INIT_SUM(sum, lf, rf, li)                                              \
  do {                                                                         \
    sum##0 = _mm256_mul_pd(lf##li, rf##0);                                     \
    sum##1 = _mm256_mul_pd(lf##li, rf##1);                                     \
    sum##2 = _mm256_mul_pd(lf##li, rf##2);                                     \
    sum##3 = _mm256_mul_pd(lf##li, rf##3);                                     \
  } while (0)
"""

def INIT_SUM(sum, lf, rf, li):
    global asm

    asm += '\n'
    for i in range(4):
        mul(dest = f'{sum[i]}', src1 = f'{lf[li]}', src2 = f'{rf[i]}')

"""
#define FUSED_TO_SUM(sum, lf, rf, li)                                          \
  do {                                                                         \
    sum##0 = _mm256_fmadd_pd(lf##li, rf##0, sum##0);                           \
    sum##1 = _mm256_fmadd_pd(lf##li, rf##1, sum##1);                           \
    sum##2 = _mm256_fmadd_pd(lf##li, rf##2, sum##2);                           \
    sum##3 = _mm256_fmadd_pd(lf##li, rf##3, sum##3);                           \
  } while (0)
"""

def FUSED_TO_SUM(sum, lf, rf, li):
    global asm

    asm += '\n'
    for i in range(4):
        fmadd(src3_dest = sum[i], src1 = lf[li], src2 = rf[i])

    pass

"""
#define WRITEBACK_TO_DEST(dest, left_y, left_base_x, sum)                      \
  do {                                                                         \
    dest->element[(left_base_x) + 0][left_y] +=                                \
        sum##0 [0] + sum##0 [1] + sum##0 [2] + sum##0 [3];                     \
    dest->element[(left_base_x) + 1][left_y] +=                                \
        sum##1 [0] + sum##1 [1] + sum##1 [2] + sum##1 [3];                     \
    dest->element[(left_base_x) + 2][left_y] +=                                \
        sum##2 [0] + sum##2 [1] + sum##2 [2] + sum##2 [3];                     \
    dest->element[(left_base_x) + 3][left_y] +=                                \
        sum##3 [0] + sum##3 [1] + sum##3 [2] + sum##3 [3];                     \
  } while (0)
"""

def WRITEBACK_TO_DEST(dest, left_y, left_base_x, sum):
    global writeback_counter, asm

    writeback_counter += 1
    
    asm += '\n'
    for extract, result in zip(tmp, sum):
        asm += f'vhaddpd {result}, {result}, {result}\n'
    for extract, result in zip(tmp, sum):
        asm += f'vextractf128 $1, {result}, {extract}\n'
    for extract, result in zip(tmp, sum):
        asm += f'addpd {ymm_to_xmm[result]}, {extract}\n'
    for i, (extract, result) in enumerate(zip(tmp, sum)):
        asm += f'vmovhpd {extract}, {right_index(left_base_x + i, left_y)}({dest})\n'

left_fragment = ['%ymm0', '%ymm1', '%ymm2', '%ymm3']
right_fragment = ['%ymm4', '%ymm5', '%ymm6', '%ymm7']
sum = ['%ymm8', '%ymm9', '%ymm10', '%ymm11']
sum_x = ['%xmm8', '%xmm9', '%xmm10', '%xmm11']
tmp = ['%xmm12', '%xmm13', '%xmm14', '%xmm15']

left = '%rdi'
right = '%rsi'
dest = '%rdx'

ymm_to_xmm = {}
for x, y in zip(sum_x, sum):
    ymm_to_xmm[y] = x

LOAD_LEFT_FRAGMENT(left_fragment, left, 0);

left_up_down_counter = 0;
left_y = 0;

while left_up_down_counter != 4:
    if True:
        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
        INIT_SUM(sum, left_fragment, right_fragment, 0);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 3);

        WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

        left_y += 1;

        LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
        INIT_SUM(sum, left_fragment, right_fragment, 3);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 0);

        WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

        left_y += 1;

        asm += f'mov ${left_y}, %ecx\n'

        while left_y < 16:
          LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
          INIT_SUM(sum, left_fragment, right_fragment, 0);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 3);

          WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

          left_y += 1;
          asm += f'inc %ecx\n'

          LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
          INIT_SUM(sum, left_fragment, right_fragment, 3);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 0);

          WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

          left_y += 1;
          asm += f'inc %ecx\n'

    left_up_down_counter += 1;

    if True:
        left_y -= 1;

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
        INIT_SUM(sum, left_fragment, right_fragment, 0);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 3);

        WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

        left_y -= 1;

        LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
        INIT_SUM(sum, left_fragment, right_fragment, 3);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

        LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
        FUSED_TO_SUM(sum, left_fragment, right_fragment, 0);

        WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

        while 0 < left_y:
          left_y -= 1;

          LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
          INIT_SUM(sum, left_fragment, right_fragment, 0);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 3);

          WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

          left_y -= 1;

          LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
          INIT_SUM(sum, left_fragment, right_fragment, 3);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 2);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 1);

          LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
          FUSED_TO_SUM(sum, left_fragment, right_fragment, 0);

          WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

    left_up_down_counter += 1;

print(f'final load is {load_counter}')

asm += '''
    ret
'''

with open('block_mult.s', 'w') as f:
    f.write(asm)
