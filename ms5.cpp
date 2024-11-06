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

typedef int64_t   VecInt64x8    __attribute__ ((vector_size(64)));
typedef int32_t   VecInt32x16   __attribute__ ((vector_size(64)));
typedef u_int64_t VecUInt64x8   __attribute__ ((vector_size(64)));
typedef u_int32_t VecUInt32x16  __attribute__ ((vector_size(64)));

union SIMD512Register {
  __m512i       d_reg;
  VecInt64x8    d_int64;
  VecInt32x16   d_int32;
  VecUInt64x8   d_uint64;
  VecUInt32x16  d_uint32;
};

void printuint64(const SIMD512Register& data) {
  printf("data: [%lu %lu %lu %lu   %lu %lu %lu %lu]\n",
    data.d_uint64[0], data.d_uint64[1], data.d_uint64[2], data.d_uint64[3],
    data.d_uint64[4], data.d_uint64[5], data.d_uint64[6], data.d_uint64[7]);
}

typedef int64_t   VecInt64x16   __attribute__ ((vector_size(128)));
typedef int32_t   VecInt32x32   __attribute__ ((vector_size(128)));
typedef u_int64_t VecUInt64x16  __attribute__ ((vector_size(128)));
typedef u_int32_t VecUInt32x32  __attribute__ ((vector_size(128)));

struct Pseudo1024Register {
  __m512i       d_reg0;
  __m512i       d_reg1;
} __attribute__((packed));

union SIMD1024Register {
  Pseudo1024Register  d_reg;
  VecInt64x16         d_int64;
  VecInt32x32         d_int32;
  VecUInt64x16        d_uint64;
  VecUInt32x32        d_uint32;
};

void printuint64(const SIMD1024Register& data) {
  printf("data: [%lu %lu %lu %lu   %lu %lu %lu %lu   %lu %lu %lu %lu   %lu %lu %lu %lu]\n",
    data.d_uint64[0], data.d_uint64[1], data.d_uint64[2], data.d_uint64[3],
    data.d_uint64[4], data.d_uint64[5], data.d_uint64[6], data.d_uint64[7],
    data.d_uint64[8], data.d_uint64[9], data.d_uint64[10], data.d_uint64[11],
    data.d_uint64[12], data.d_uint64[13], data.d_uint64[14], data.d_uint64[15]);
}

const int CAP = 2;

u_int64_t d0[4] = {2,4,5,7};
u_int64_t d1[4] = {0,3,6,9};
u_int64_t d2[4] = {1,2,3,4};
u_int64_t d3[4] = {0,0,0,0};
u_int64_t d4[4] = {10,10,10,10};
u_int64_t d5[4] = {9,9,10,10};
u_int64_t d6[4] = {1,1,1,1};
u_int64_t d7[4] = {3,3,3,3};

alignas(64)
u_int64_t *dataIn[8] = {d0,d1,d2,d3, d4,d5,d6,d7};

alignas(64)
u_int64_t data0[64];
alignas(64)
u_int64_t data1[64];
u_int64_t sorts=0;

void merge(const int size, u_int64_t *a, u_int64_t *b, u_int64_t *c) {
  const u_int64_t *aend = a+size;
  const u_int64_t *bend = b+size;

  #pragma GCC unroll 4
  while (a!=aend && b!=bend) {
    if (((*b)&(0xffffffffUL))<((*a)&0xffffffffUL)) {
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

void mergeSimd(const int size, u_int64_t *a, u_int64_t *b, u_int64_t *c) {
  SIMD512Register lhs, rhs;
  SIMD1024Register dst;
  lhs.d_reg = _mm512_load_epi64(a);
  rhs.d_reg = _mm512_load_epi64(b);
  int32_t aidx = 0;
  int32_t bidx = 0;
  int32_t cidx = 0;

  #pragma GCC unroll 4
  while (aidx!=size && bidx!=size) {
    if (rhs.d_int32[bidx<<1]<lhs.d_int32[aidx<<1]) {
      dst.d_int64[cidx++] = rhs.d_int64[bidx++];
    } else {
      dst.d_int64[cidx++] = lhs.d_int64[aidx++];
    }
  }

  while (aidx!=size) {
    dst.d_int64[cidx++] = lhs.d_int64[aidx++];
  }
  while (bidx!=size) {
    dst.d_int64[cidx++] = rhs.d_int64[bidx++];
  }

  _mm512_store_epi64(c,   dst.d_reg.d_reg0);
  _mm512_store_epi64(c+8, dst.d_reg.d_reg1);
}

void sort() {
  // merge d0,d1 output 4 elems
  merge(CAP, data0+0, data0+8,   data1);
  // merge d2,d3 output 4 elems
  merge(CAP, data0+16, data0+24, data1+(CAP<<1));
  // merge d4,d5 output 4 elems
  merge(CAP, data0+32, data0+40, data1+(CAP<<2));
  // merge d5,d6 output 4 elems
  merge(CAP, data0+48, data0+56, data1+(CAP<<1)+(CAP<<2));

  // merge d0d1,d2d3 output 8 elems
  merge(CAP<<1, data1, data1+(CAP<<1), data0);
  // merge d4d5,d6d7 outout 8 elems
  merge(CAP<<1, data1+(CAP<<2), data1+(CAP<<1)+(CAP<<2), data0+8);

  // d0d1d2d3 + d4d5d6d7 output 16 elems
  mergeSimd(CAP<<2, data0, data0+8, data1);

#ifndef NDEBUG
  printf("Sorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data1[i]!=-1) {
      printf("%d=%u/%u ", i, (u_int32_t)data1[i], (u_int32_t)(data1[i]>>32));
    }
  }
  printf("\n");
#endif
}

void prepare(u_int64_t *d0, u_int64_t *d1, u_int64_t *d2, u_int64_t *d3,
             u_int64_t *d4, u_int64_t *d5, u_int64_t *d6, u_int64_t *d7) {
  u_int64_t n=0x13432;
  for (int i=0; i<64; ++i, ++n) {
    data0[i]=-1;
  }
  
  int b = 0;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d0[i]|(n++<<32);
  }

  b = 8;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d1[i]|(n++<<32);
  }

  b = 16;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d2[i]|(n++<<32);
  }

  b = 24;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d3[i]|(n++<<32);
  }

  b = 32;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d4[i]|(n++<<32);
  }

  b = 40;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d5[i]|(n++<<32);
  }

  b = 48;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d6[i]|(n++<<32);
  }

  b = 56;
  for (int i=0; i<CAP; ++i) {
    data0[b+i] = d7[i]|(n++<<32);
  }

#ifndef NDEBUG
  printf("Unsorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data0[i]!=-1) {
      printf("%d=%u/%u ", i, (u_int32_t)data0[i], (u_int32_t)(data0[i]>>32));
    }
  }
  printf("\n\n");
#endif
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
