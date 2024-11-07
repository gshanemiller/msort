#!/usr/bin/python3.12

# Need 16 slots (CAP=2 * 8 threads)
#
# zmm16, zmm17, zmm18 sratch
# ======================================
# 2+2 -> 4 zmm19
# 2+2 -> 4 zmm20
# 2+2 -> 4 zmm21
# 2+2 -> 4 zmm22
#
# 4+4 -> 8 zmm19+zmm20 -> zmm23
# 4+4 -> 8 zmm21+zmm22 -> zmm24
#
# 8+8 -> 16 zmm23+zmm24 -> zmm25+zmm26

CODE='''
inline
void merge4(u_int64_t *lhs, u_int64_t *rhs, u_int64_t *dst) {
  asm("mov %0, %%rdx;"
      "vmovdqa (%%rdx), %%xmm0;"
      "vmovdqa (%%rdx), %%xmm1;"
      "vmovq %%xmm0, %%r8;"               // lhs[0]
      "psrldq $8, %%xmm0;"
      "vmovq %%xmm0, %%r9;"               // lhs[1]
      "vmovq %%xmm1, %%r10;"              // rhs[0]
      "psrldq $8, %%xmm1;"
      "vmovq %%xmm1, %%r11;"              // rhs[1]
      "vmovq %%r8, %%xmm2;"               // store lhs[0]

      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm2, %%xmm2;"      // combine 4+2 in 2
      "vmovq %%r10, %%xmm3;"              // store rhs[0]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+3 in 3

      "vmovq %%r10, %%xmm4;"              // store rhs[0]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm2, %%xmm2;"      // combine 4+2 in 2

      "vmovq %%r9, %%xmm3;"               // store lhs[1]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+2 in 2

      "vmovq %%r11, %%xmm3;"              // store rhs[1]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+2 in 2

      "vmovq %%r10, %%xmm2;"              // store rhs[0]
      "vmovq %%r8, %%xmm4;"               // store lhs[0]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm2, %%xmm2;"      // combine 4+2 in 2

      "vmovq %%r9, %%xmm3;"               // store lhs[1]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+3 in 3

      "vmovq %%r11, %%xmm3;"              // store rhs[1]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+3 in 3

      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm2, %%xmm2;"      // combine 4+2 in 2
      "vmovq %%r8, %%xmm3;"               // store lhs[0]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpor %%xmm4, %%xmm3, %%xmm3;"      // combine 4+3 in 3
      :
      : "m"(lhs),
        "m"(rhs),
        "m"(dst)
      :
  );
}
'''

def replace(search, replace):
  s = CODE
  i = 0
  while i<len(search):
    s = s.replace(search[i], replace[i])
    i=i+1
  print(s)

replace(["void merge4",       "%xmm0", "%xmm1", "%xmm4", "%xmm2", "%xmm3"],
        ["void merge4_zmm19", "%zmm16","%zmm17","%zmm18","%zmm19","%zmm19"])

# replace(["void merge4",       "%xmm0", "%xmm1", "%xmm4", "%xmm2", "%xmm3"],
#         ["void merge4_zmm20", "%zmm16","%zmm17","%zmm18","%zmm20","%zmm20"])

# replace(["void merge4",       "%xmm0", "%xmm1", "%xmm4", "%xmm2", "%xmm3"],
#         ["void merge4_zmm21", "%zmm16","%zmm17","%zmm18","%zmm21","%zmm21"])

# replace(["void merge4",       "%xmm0", "%xmm1", "%xmm4", "%xmm2", "%xmm3"],
#         ["void merge4_zmm22", "%zmm16","%zmm17","%zmm18","%zmm22","%zmm22"])
