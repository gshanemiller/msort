#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <immintrin.h>

typedef int64_t   VecInt64x2 __attribute__ ((vector_size(16)));
typedef int32_t   VecInt32x4 __attribute__ ((vector_size(16)));
typedef u_int64_t VecUInt64x2 __attribute__ ((vector_size(16)));
typedef u_int32_t VecUInt32x4 __attribute__ ((vector_size(16)));

union SIMD128Register {
  __m128i     d_reg;
  VecInt64x2  d_int64;
  VecInt32x4  d_int32;
  VecUInt64x2 d_uint64;
  VecUInt32x4 d_uint32;
};

void printuint64(const SIMD128Register& data) {
  printf("data: [%lu %lu]\n", data.d_uint64[0], data.d_uint64[1]);
}

typedef int64_t   VecInt64x4  __attribute__ ((vector_size(32)));
typedef int32_t   VecInt32x8  __attribute__ ((vector_size(32)));
typedef u_int64_t VecUInt64x4 __attribute__ ((vector_size(32)));
typedef u_int32_t VecUInt32x8 __attribute__ ((vector_size(32)));

union SIMD256Register {
  __m256i     d_reg;
  VecInt64x4  d_int64;
  VecInt32x8  d_int32;
  VecUInt64x4 d_uint64;
  VecUInt32x8 d_uint32;
};

void printuint64(const SIMD256Register& data) {
  printf("data: [%lu %lu %lu %lu]\n", data.d_uint64[0], data.d_uint64[1], data.d_uint64[2], data.d_uint64[3]);
}

const int CAP = 2;

u_int64_t d0[4] = {1, 2, 3, 4};
u_int64_t d1[4] = {0, 0, 0, 0};
u_int64_t d2[4] = {10, 10, 10, 10};
u_int64_t d3[4] = {9,9,10,10};
u_int64_t d4[4] = {1,1,1,1};
u_int64_t d5[4] = {3,3,3,3};
u_int64_t d6[4] = {6,9,11,12};
u_int64_t d7[4] = {2,4,5,7};

u_int64_t *dataIn[8] = {d0,d1,d2,d3, d4,d5,d6,d7};

u_int64_t data0[64];
u_int64_t data1[64];
u_int64_t sorts=0;

// Merge-sort at most 2 elements from 'lhs', and at most 2 from 'rhs'
// Input:   lhs xmm0, rhs xmm1
// Output:  hv1 xmm2, hv2 xmm3
void merge4(u_int64_t *lhs, u_int64_t *rhs, u_int64_t *dst) {
  asm("mov %0, %%rdx;"
      "vmovdqa (%%rdx), %%xmm0;"
      "mov %1, %%rdx;"
      "vmovdqa (%%rdx), %%xmm1;"
      "vmovq %%xmm0, %%r8;"               // lhs[0]
      "psrldq $8, %%xmm0;" 
      "vmovq %%xmm0, %%r9;"               // lhs[1]
      "vmovq %%xmm1, %%r10;"              // rhs[0]
      "psrldq $8, %%xmm1;" 
      "vmovq %%xmm1, %%r11;"              // rhs[1]

      "branch1%=:;"                       // branch 1
      "cmp %%r10d, %%r8d;"                // cmp lhs[0], rhs[0]
      "jge branch4%=;"
      "vmovq %%r8, %%xmm2;"               // store lhs[0]

      "branch2%=:;"                       // branch 2
      "cmp %%r10d, %%r9d;"                // cmp lhs[1], rhs[0]
      "jge branch3%=;"
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm2, %%xmm2;"    // combine 4+2 in 2
      "vmovq %%r10, %%xmm3;"              // store rhs[0]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm3, %%xmm3;"    // combine 4+3 in 3
      "jmp done%=;"                       // all done!

      "branch3%=:;"                       // branch 3
      "vmovq %%r11, %%xmm4;"              // store rhs[0]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm2, %%xmm2;"    // combine 4+2 in 2

      "cmp %%r11d, %%r9d;"                // cmp lhs[1], rhs[1]
      "jge endbranch3%=;"
      "vmovq %%r9, %%xmm3;"               // store lhs[1]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm3, %%xmm3;"    // combine 4+2 in 2
      "jmp done%=;"                       // all done!

      "endbranch3%=:;"                    // branch 3 else part
      "vmovq %%r11, %%xmm3;"              // store rhs[1]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm3, %%xmm3;"    // combine 4+2 in 2
      "jmp done%=;"                       // all done!

      "branch4%=:;"                       // branch 4
      "vmovq %%r10, %%xmm2;"              // store rhs[0]
      "cmp %%r11d, %%r8d;"                // compare lhs[0], rhs[1]
      "jge endbranch4%=;"
      "vmovq %%r8, %%xmm4;"               // store lhs[0]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm2, %%xmm2;"    // combine 4+2 in 2

      "branch5%=:;"                       // branch 5
      "cmp %%r11d, %%r9d;"                // compare lhs[1], rhs[1]
      "jge endbranch5%=;"
      "vmovq %%r9, %%xmm3;"               // store lhs[1]
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm3, %%xmm3;"    // combine 4+3 in 3
      
      "endbranch5%=:;"                    // else part of branch 5
      "vmovq %%r11, %%xmm3;"              // store rhs[1]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm3, %%xmm3;"    // combine 4+3 in 3
      "jmp done%=;"                       // all done!

      "endbranch4%=:;"                    // else part of branch 4
      "vmovq %%r11, %%xmm4;"              // store rhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm2, %%xmm2;"    // combine 4+2 in 2
      "vmovq %%r8, %%xmm3;"               // store lhs[0]
      "vmovq %%r9, %%xmm4;"               // store lhs[1]
      "pslldq $8, %%xmm4;"
      "vpaddd %%xmm4, %%xmm2, %%xmm3;"    // combine 4+3 in 3

      "done%=:;"                          // store sorted data
      "mov %2, %%rdx;"                    // dst
      "vmovdqa %%xmm2, (%%rdx);"          // copy xmm2 to *dst
      "add $16, %%rdx;"                   // advance 16 bytes
      "vmovdqa %%xmm3, (%%rdx);"          // copy xmm3 to *dst
      :
      : "m"(lhs),
        "m"(rhs),
        "m"(dst)
      : "%rdx",
        "%r8",
        "%r9",
        "%r10",
        "%r11",
        "%xmm0",
        "%xmm1",
        "%xmm2",
        "%xmm3"
  );
}

void merge(const int size, u_int64_t *a, u_int64_t *b, u_int64_t *c) {
  const u_int64_t *aend = a+size;
  const u_int64_t *bend = b+size;

  while (a!=aend && b!=bend) {
    if (*b<*a) {
      *c++ = *b++;
    } else {
      *c++ = *a++;
    }
  }

  while (a!=aend) {
    *c++ = *a++;
  }
  while (b!=bend) {
    *c++ = *b++;
  }
}

void sort() {
  // merge d0,d1 into data+0    using 16  end 16
  merge4(data0+0, data0+8, data1);
  // merge d2,d3 into data+16   using 16  end 32
  merge4(data0+16, data0+24, data1+16);
  // merge d4,d5 into data+32   using 16  end 48
  merge4(data0+32, data0+40, data1+32);
  // merge d5,d6 into data+48   using 16  end 64
  merge4(data0+48, data0+48, data1+48);

  // merge d0d1+0 d2d3+16 into dataIn+0 using 32
  merge(CAP<<1, data1+0, data1+16, data0);
  // merge d4d5+32 d6d7+48 into dataIn+32 using 32
  merge(CAP<<1, data1+32, data1+48, data0+32);

  // final merge
  merge(CAP<<2, data0+0, data0+32, data1);

/*
  printf("Sorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data1[i]!=-1) {
      printf("%d=%d ", i, data1[i]);
    }
  }
  printf("\n");
*/
}

void prepare(u_int64_t *d0, u_int64_t *d1, u_int64_t *d2, u_int64_t *d3,
             u_int64_t *d4, u_int64_t *d5, u_int64_t *d6, u_int64_t *d7) {
  for (int i=0; i<64; ++i) {
    data0[i]=-1;
  }
  
  int b = 0;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d0[i];
  }

  b = 8;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d1[i];
  }

  b = 16;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d2[i];
  }

  b = 24;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d3[i];
  }

  b = 32;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d4[i];
  }

  b = 40;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d5[i];
  }

  b = 48;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d6[i];
  }

  b = 56;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d7[i];
  }

/*
  printf("Unsorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data0[i]!=-1) {
      printf("%d=%d ", i, data0[i]);
    }
  }
  printf("\n\n");
*/
}

int main() {
  timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i0=0; i0<8; ++i0) {
  for (int i1=0; i1<8; ++i1) {
  for (int i2=0; i2<8; ++i2) {
  for (int i3=0; i3<8; ++i3) {
  for (int i4=0; i4<8; ++i4) {
  for (int i5=0; i5<8; ++i5) {
  for (int i6=0; i6<8; ++i6) {
  for (int i7=0; i7<8; ++i7) {
    prepare(dataIn[i0], dataIn[i1], dataIn[i2], dataIn[i3],
            dataIn[i4], dataIn[i5], dataIn[i6], dataIn[i7]);
    sort();
    ++sorts;
  }
  }
  }
  }
  }
  }
  }
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  u_int64_t nsDiff = (end.tv_sec-start.tv_sec)*1000000000UL + end.tv_nsec - start.tv_nsec;
  double nsPerSort = ((nsDiff*1.0)/(sorts*1.0));
  printf("raw  : cap %d sorts %lu %.0lf nsPerSort %lf million sorts/sec\n", CAP, sorts, nsPerSort, 1000.0/nsPerSort);
  nsPerSort -= 12.0;
  printf("final: cap %d sorts %lu %.0lf nsPerSort %lf million sorts/sec\n", CAP, sorts, nsPerSort, 1000.0/nsPerSort);

  return 0;
}
