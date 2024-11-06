#include <stdio.h>
#include <time.h>
#include <sys/types.h>

const int CAP = 2;

u_int64_t d0[4] = {2,4,5,7};
u_int64_t d1[4] = {0,3,6,9};
u_int64_t d2[4] = {1,2,3,4};
u_int64_t d3[4] = {0,0,0,0};
u_int64_t d4[4] = {10,10,10,10};
u_int64_t d5[4] = {9,9,10,10};
u_int64_t d6[4] = {1,1,1,1};
u_int64_t d7[4] = {3,3,3,3};

u_int64_t *dataIn[8] = {d0,d1,d2,d3, d4,d5,d6,d7};

u_int64_t data0[64];
u_int64_t data1[64];

u_int64_t sorts=0;

void merge(const int size, u_int64_t *a, u_int64_t *b, u_int64_t *c) {
  const u_int64_t *aend = a+size;
  const u_int64_t *bend = b+size;

  #pragma GCC unroll 4
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

  // d0d1d2d3 + d4d5d6d7 output 16 elem
  merge(CAP<<2, data0, data0+8, data1);

#ifndef NDEBUG
  printf("Sorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<CAP*8; ++i) {
    if (data1[i]!=-1) {
      printf("%d=%lu ", i, data1[i]);
    }
  }
  printf("\n");
#endif
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

#ifndef NDEBUG
  printf("Unsorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data0[i]!=-1) {
      printf("%d=%lu ", i, data0[i]);
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
