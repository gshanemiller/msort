#include <stdio.h>
#include <time.h>
#include <sys/types.h>

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

inline
void merge1(int size, int *a, int *b, int *c) {
  int *aend = a+size;
  int *bend = b+size;
  asm("mov %0, %%rax;"          // aend
      "mov %1, %%rbx;"          // bend
      "mov %2, %%rcx;"          // a
      "mov %3, %%rdx;"          // b
      "mov %4, %%rdi;"          // c
      "xor %%r8, %%r8;"         // zero r8 holding data for compare
      "xor %%r9, %%r9;"         // zero r9 holding data for compare
      "loop%=:;"                // main loop start
      "cmp %%rax, %%rcx;"       // does aend==a?
      "je done%=;"              // yes, exit loop
      "cmp %%rbx, %%rdx;"       // does bend==b?
      "je done%=;"              // yes, exit loop
      "mov (%%rdx), %%r8d;"     // set r8d = *b (hi 32 bits always 0)
      "mov (%%rcx), %%r9d;"     // set r9d = *a (hi 32 bits always 0)
      "xor %%r10, %%r10;"       // zero r10
      "xor %%r11, %%r11;"       // zero r11
      "cmp %%r9d, %%r8d;"       // compare for *b<*a
      "setl %%r10b;"            // put less-than bit into r10b (1 if *b<*a  else 0)
      "setge %%r11b;"           // put gt-or-eq  bit into r11b (1 if *b>=*a else 0)
      "mov %%r10, %%r12;"       // copy r10 to r12
      "mov %%r11, %%r13;"       // copy r11 to r13
                                // Compute two's complenent on r12, r13: (0,1) -> (0, -1)
      "not %%r12;"              // twos-comp 1 of 2
      "add $1, %%r12;"          // twos-comp 2 of 2
      "not %%r13;"              // twos-comp 1 of 2
      "add $1, %%r13;"          // twos-comp 2 of 2
      "and %%r12, %%r8;"        // and
      "and %%r13, %%r9;"        // and
      "add %%r8, %%r9;"         // value to be stored
      "mov %%r9, (%%rdi);"      // do *c = *r9
      "add $4, %%rdi;"          // advance 'c' one integer
      "shl $2, %%r10;"          // multiply r10 by 4 e.g. 1 integer
      "shl $2, %%r11;"          // multiply r11 by 4 e.g. 1 integer
      "add %%r10, %%rdx;"       // increment (or not) b
      "add %%r11, %%rcx;"       // increment (or not) a
      "jmp loop%=;"             // do more more
      "done%=:;"                // main loop done
      :
      : "m"(aend),
        "m"(bend),
        "m"(a),
        "m"(b),
        "m"(c)
      : "%rax",
        "%rbx",
        "%rcx",
        "%rdx",
        "%rdi",
        "%r8",
        "%r9",
        "%r10",
        "%r11",
        "%r12",
        "%r13"
  );
  while (a!=aend) {
    *c++ = *a++;
  }
  while (b!=bend) {
    *c++ = *b++;
  }
}

void sort() {
  // merge d0,d1 into data+0    using 16  end 16
  merge1(4, data0+0, data0+8, data1);
  // merge d2,d3 into data+16   using 16  end 32
  merge1(4, data0+16, data0+24, data1+16);
  // merge d4,d5 into data+32   using 16  end 48
  merge1(4, data0+32, data0+40, data1+32);
  // merge d5,d6 into data+48   using 16  end 64
  merge1(4, data0+48, data0+48, data1+48);

  // merge d0d1+0 d2d3+16 into dataIn+0 using 32
  merge1(8, data1+0, data1+16, data0);
  // merge d4d5+32 d6d7+48 into dataIn+32 using 32
  merge1(8, data1+32, data1+48, data0+32);

  // final merge
  merge1(16, data0+0, data0+32, data1);

/*
  printf("Sorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<64; ++i) {
    if (data1[i]!=-1) {
      printf("%d=%d ", i, data1[i]);
    }
  }
  printf("\n");
  printf("\n");
*/
}

void prepare(int *d0, int *d1, int *d2, int *d3,
             int *d4, int *d5, int *d6, int *d7) {
  for (int i=0; i<64; ++i) {
    data0[i]=-1;
  }
  
  int b = 0;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d0[i];
  }

  b = 8;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d1[i];
  }

  b = 16;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d2[i];
  }

  b = 24;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d3[i];
  }

  b = 32;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d4[i];
  }

  b = 40;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d5[i];
  }

  b = 48;
  for (int i=0; i<4; ++i) {
    data0[b+i] = d6[i];
  }

  b = 56;
  for (int i=0; i<4; ++i) {
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
  printf("\n");
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
  printf("raw  : sorts %u %.0lf nsPerSort %lf million sorts/sec\n", sorts, nsPerSort, 1000.0/nsPerSort);
  nsPerSort -= 12.0;
  printf("final: sorts %u %.0lf nsPerSort %lf million sorts/sec\n", sorts, nsPerSort, 1000.0/nsPerSort);

  return 0;
}
