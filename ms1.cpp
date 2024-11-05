#include <stdio.h>
#include <time.h>
#include <sys/types.h>

const int CAP = 2;

int d0[4] = {1, 2, 3, 4};
int d1[4] = {0, 0, 0, 0};
int d2[4] = {10, 10, 10, 10};
int d3[4] = {9,9,10,10};
int d4[4] = {1,1,1,1};
int d5[4] = {3,3,3,3};
int d6[4] = {6,9,11,12};
int d7[4] = {2,4,5,7};

int *dataIn[8] = {d0,d1,d2,d3, d4,d5,d6,d7};

int data0[64];
int data1[64];

unsigned sorts=0;

void merge(const int size, int *a, int *b, int *c) {
  int i=0;
  int j=0;
  int writeIdx = 0;

  while (i<size && j<size) { 
    if (b[j]<a[i]) {
      c[writeIdx++] = b[j++]; 
    } else {
      c[writeIdx++] = a[i++]; 
    }
  }

  while (i<size) {
    c[writeIdx++] = a[i++]; 
  }
  while (j<size) {
    c[writeIdx++] = b[j++]; 
  }
}

void sort() {
  // merge d0,d1 into data+0    using 16  end 16
  merge(CAP, data0+0, data0+8, data1);
  // merge d2,d3 into data+16   using 16  end 32
  merge(CAP, data0+16, data0+24, data1+16);
  // merge d4,d5 into data+32   using 16  end 48
  merge(CAP, data0+32, data0+40, data1+32);
  // merge d5,d6 into data+48   using 16  end 64
  merge(CAP, data0+48, data0+48, data1+48);

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

void prepare(int *d0, int *d1, int *d2, int *d3,
             int *d4, int *d5, int *d6, int *d7) {
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
  printf("raw  : cap %d sorts %u %.0lf nsPerSort %lf million sorts/sec\n", CAP, sorts, nsPerSort, 1000.0/nsPerSort);
  nsPerSort -= 12.0;
  printf("final: cap %d sorts %u %.0lf nsPerSort %lf million sorts/sec\n", CAP, sorts, nsPerSort, 1000.0/nsPerSort);

  return 0;
}
