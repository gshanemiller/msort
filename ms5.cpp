#include <stdio.h>
#include <time.h>
#include <sys/types.h>

const int CAP = 2;

u_int64_t d0[4] = {7,5,4,2};
u_int64_t d1[4] = {9,6,3,0};
u_int64_t d2[4] = {4,3,2,1};
u_int64_t d3[4] = {0,0,0,0};
u_int64_t d4[4] = {10,10,10,10};
u_int64_t d5[4] = {9,9,9,9};
u_int64_t d6[4] = {1,1,1,1};
u_int64_t d7[4] = {3,3,3,3};

alignas(64)
u_int64_t *dataIn[8] = {d0,d1,d2,d3, d4,d5,d6,d7};

alignas(64)
u_int64_t data0[64];
alignas(64)
u_int64_t data1[64];

alignas(64)
u_int64_t rrot[8]={7,0,1,2,3,4,5,6}; // right rotate by one uint64
u_int64_t *rrotPtr(rrot+0);
alignas(64)
u_int64_t lrot[8]={1,2,3,4,5,6,7,0}; // left rotate by one uint64
u_int64_t *lrotPtr(lrot+0);

u_int64_t sorts=0;

// zmm16, zmm17, zmm30, zmm31
// ======================================                                                                                
// 2+2 -> 4 zmm19                                                                                                        
// 2+2 -> 4 zmm20                                                                                                        
// 2+2 -> 4 zmm21                                                                                                        
// 2+2 -> 4 zmm22                                                                                                        
// 
// 4+4 -> 8 zmm19+zmm20 -> zmm23                                                                                         
// 4+4 -> 8 zmm21+zmm22 -> zmm24                                                                                         
// 8+8 -> 16 zmm23+zmm24 -> zmm25+zmm26  

void simd_init() {
  asm("mov %0, %%r8;"                             // load right rotate addr
      "vmovdqa64 (%%r8), %%zmm30;"                // load *rrot
      "mov %1, %%r8;"                             // load left rotate addr
      "vmovdqa64 (%%r8), %%zmm31;"                // load *lrol
      "mov $3, %%r8d;"                            // mov 3 into r8d - OR mask bottom two int32s
      "kmovw %%r8d, %%k2;"                        // set k2 to 3
      :
      :
        "m"(rrotPtr),
        "m"(lrotPtr)
      :
  );
}

void merge4_zmm18(u_int64_t *lhs, u_int64_t *rhs) {
  asm("mov %0, %%r8;"                             // load lhs addr
      "vmovdqa64 (%%r8), %%zmm16;"                // load *lhs
      "mov %1, %%r8;"                             // load rhs addr
      "vmovdqa64 (%%r8), %%zmm17;"                // load *rhs
      "mov $2, %%r8;"                             // lhs count
      "mov $2, %%r9;"                             // rhs count
      "vpxorq %%zmm18, %%zmm18, %%zmm18;"         // zero zmm18 (merge result goes here)

      "loop1%=:;"                                 // loop 1

      "vpcmpd $5,%%zmm16, %%zmm17, %%k1;"         // zmm17 (rhs) > zmm16 (lhs)?
      "kmovw %%k1, %%r10d;"                       // copy k1 cmp mask to r10d
      "and $1, %%r10d;"                           // isolate 1 bit - only care about zmm17[0]<zmm16[0] @ int32
      "cmp $1, %%r10d;"                           // is rb10==1?
      "je rhsless%=;"                             // rhs less (otherwise lhs less)

      "lhsless%=:;"                               // lhs less branch
      "vpermq %%zmm18, %%zmm30, %%zmm18;"         // right rotate by uint64
      "vpord %%zmm16, %%zmm18, %%zmm18%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm16, %%zmm31, %%zmm16;"         // left rotate by uint64
      "dec %%r8b;"                                // lhscount -= 1 
      "jmp bottomloop1%=;"                        // see if more work
      
      "rhsless%=:;"                               // rhs branch
      "vpermq %%zmm18, %%zmm30, %%zmm18;"         // right rotate by uint64
      "vpord %%zmm17, %%zmm18, %%zmm18%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm17, %%zmm31, %%zmm17;"         // left rotate by uint64
      "dec %%r9b;"                                // rhscount -= 1 
      
      "bottomloop1%=:;"                           // book keeping on counts/dst
      "cmp $0, %%r8b;"                            // compare lhscount r8b to 0
      "setg %%r10b;"                              // set r10b to hold GT flag from cmp 
      "cmp $0, %%r9b;"                            // compare rhscount r9b to 0
      "setg %%r11b;"                              // set r10b to hold GT flag from cmp 
      "shl $1, %%r11b;"                           // move GT flag over one bit
      "or %%r11b, %%r10b;"                        // combine GT flags
      "cmp $3, %%r10b;"                           // is r10b 3?
      "je loop1%=;"                               // if yes, more loop1 work
      "cmp $0, %%r8b;"                            // was lhs count 0?
      "je rhstailloop%=;"                         // do rhs work

      "lhstailloop%=:;"                           // lhs tail work
      "vpermq %%zmm18, %%zmm30, %%zmm18;"         // right rotate by uint64
      "vpord %%zmm16, %%zmm18, %%zmm18%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm16, %%zmm31, %%zmm16;"         // left rotate by uint64
      "sub $1, %%r8b;"                            // lhscount -= 1 
      "jnz lhstailloop%=;"                        // do more lhs tail work
      "jmp done%=;"                               // all done

      "rhstailloop%=:;"                           // rhs tail work
      "vpermq %%zmm18, %%zmm30, %%zmm18;"         // right rotate by uint64
      "vpord %%zmm17, %%zmm18, %%zmm17%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm17, %%zmm31, %%zmm17;"         // left rotate by uint64
      "sub $1, %%r9b;"                            // lhscount -= 1 
      "jnz rhstailloop%=;"                        // do more rhs tail work
      
      "done%=:;"                                  // all done
      :
      : "m"(lhs),
        "m"(rhs)
      :
  );
}

void merge8_zmm19_20() {
  asm("mov $4, %%r8;"                             // lhs count
      "mov $4, %%r9;"                             // rhs count
      "vpxorq %%zmm23, %%zmm23, %%zmm23;"         // zero zmm18 (merge result goes here)

      "loop1%=:;"                                 // loop 1

      "vpcmpd $5,%%zmm19, %%zmm20, %%k1;"         // zmm17 (rhs) > zmm16 (lhs)?
      "kmovw %%k1, %%r10d;"                       // copy k1 cmp mask to r10d
      "and $1, %%r10d;"                           // isolate 1 bit - only care about zmm17[0]<zmm16[0] @ int32
      "cmp $1, %%r10d;"                           // is rb10==1?
      "je rhsless%=;"                             // rhs less (otherwise lhs less)

      "lhsless%=:;"                               // lhs less branch
      "vpermq %%zmm23, %%zmm30, %%zmm23;"         // right rotate by uint64
      "vpord %%zmm19, %%zmm23, %%zmm23%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm19, %%zmm31, %%zmm19;"         // left rotate by uint64
      "dec %%r8b;"                                // lhscount -= 1 
      "jmp bottomloop1%=;"                        // see if more work
      
      "rhsless%=:;"                               // rhs branch
      "vpermq %%zmm23, %%zmm30, %%zmm23;"         // right rotate by uint64
      "vpord %%zmm20, %%zmm23, %%zmm23%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm20, %%zmm31, %%zmm20;"         // left rotate by uint64
      "dec %%r9b;"                                // rhscount -= 1 
      
      "bottomloop1%=:;"                           // book keeping on counts/dst
      "cmp $0, %%r8b;"                            // compare lhscount r8b to 0
      "setg %%r10b;"                              // set r10b to hold GT flag from cmp 
      "cmp $0, %%r9b;"                            // compare rhscount r9b to 0
      "setg %%r11b;"                              // set r10b to hold GT flag from cmp 
      "shl $1, %%r11b;"                           // move GT flag over one bit
      "or %%r11b, %%r10b;"                        // combine GT flags
      "cmp $3, %%r10b;"                           // is r10b 3?
      "je loop1%=;"                               // if yes, more loop1 work
      "cmp $0, %%r8b;"                            // was lhs count 0?
      "je rhstailloop%=;"                         // do rhs work

      "lhstailloop%=:;"                           // lhs tail work
      "vpermq %%zmm23, %%zmm30, %%zmm23;"         // right rotate by uint64
      "vpord %%zmm19, %%zmm23, %%zmm23%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm19, %%zmm31, %%zmm19;"         // left rotate by uint64
      "sub $1, %%r8b;"                            // lhscount -= 1 
      "jnz lhstailloop%=;"                        // do more lhs tail work
      "jmp done%=;"                               // all done

      "rhstailloop%=:;"                           // rhs tail work
      "vpermq %%zmm23, %%zmm30, %%zmm23;"         // right rotate by uint64
      "vpord %%zmm20, %%zmm23, %%zmm20%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm20, %%zmm31, %%zmm20;"         // left rotate by uint64
      "sub $1, %%r9b;"                            // lhscount -= 1 
      "jnz rhstailloop%=;"                        // do more rhs tail work
      
      "done%=:;"                                  // all done
      :
      :
      :
  );
}

void merge8_zmm21_22() {
  asm("mov $4, %%r8;"                             // lhs count
      "mov $4, %%r9;"                             // rhs count
      "vpxorq %%zmm24, %%zmm24, %%zmm24;"         // zero zmm18 (merge result goes here)

      "loop1%=:;"                                 // loop 1

      "vpcmpd $5,%%zmm21, %%zmm22, %%k1;"         // zmm17 (rhs) > zmm16 (lhs)?
      "kmovw %%k1, %%r10d;"                       // copy k1 cmp mask to r10d
      "and $1, %%r10d;"                           // isolate 1 bit - only care about zmm17[0]<zmm16[0] @ int32
      "cmp $1, %%r10d;"                           // is rb10==1?
      "je rhsless%=;"                             // rhs less (otherwise lhs less)

      "lhsless%=:;"                               // lhs less branch
      "vpermq %%zmm24, %%zmm30, %%zmm24;"         // right rotate by uint64
      "vpord %%zmm21, %%zmm24, %%zmm24%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm21, %%zmm31, %%zmm21;"         // left rotate by uint64
      "dec %%r8b;"                                // lhscount -= 1 
      "jmp bottomloop1%=;"                        // see if more work
      
      "rhsless%=:;"                               // rhs branch
      "vpermq %%zmm24, %%zmm30, %%zmm24;"         // right rotate by uint64
      "vpord %%zmm22, %%zmm24, %%zmm24%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm22, %%zmm31, %%zmm22;"         // left rotate by uint64
      "dec %%r9b;"                                // rhscount -= 1 
      
      "bottomloop1%=:;"                           // book keeping on counts/dst
      "cmp $0, %%r8b;"                            // compare lhscount r8b to 0
      "setg %%r10b;"                              // set r10b to hold GT flag from cmp 
      "cmp $0, %%r9b;"                            // compare rhscount r9b to 0
      "setg %%r11b;"                              // set r10b to hold GT flag from cmp 
      "shl $1, %%r11b;"                           // move GT flag over one bit
      "or %%r11b, %%r10b;"                        // combine GT flags
      "cmp $3, %%r10b;"                           // is r10b 3?
      "je loop1%=;"                               // if yes, more loop1 work
      "cmp $0, %%r8b;"                            // was lhs count 0?
      "je rhstailloop%=;"                         // do rhs work

      "lhstailloop%=:;"                           // lhs tail work
      "vpermq %%zmm24, %%zmm30, %%zmm24;"         // right rotate by uint64
      "vpord %%zmm21, %%zmm24, %%zmm24%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm21, %%zmm31, %%zmm21;"         // left rotate by uint64
      "sub $1, %%r8b;"                            // lhscount -= 1 
      "jnz lhstailloop%=;"                        // do more lhs tail work
      "jmp done%=;"                               // all done

      "rhstailloop%=:;"                           // rhs tail work
      "vpermq %%zmm24, %%zmm30, %%zmm24;"         // right rotate by uint64
      "vpord %%zmm22, %%zmm24, %%zmm22%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm22, %%zmm31, %%zmm22;"         // left rotate by uint64
      "sub $1, %%r9b;"                            // lhscount -= 1 
      "jnz rhstailloop%=;"                        // do more rhs tail work
      
      "done%=:;"                                  // all done
      :
      :
      :
  );
}

void merge16() {
  asm("mov $8, %%r8;"                             // lhs count
      "mov $8, %%r9;"                             // rhs count
      "mov $0, %%r11;"                            // dst count
      "vpxorq %%zmm25, %%zmm25, %%zmm25;"         // zero zmm25 (merge result goes here)

      "loop1%=:;"                                 // loop 1

      "vpcmpd $5,%%zmm23, %%zmm24, %%k1;"         // zmm17 (rhs) > zmm16 (lhs)?
      "kmovw %%k1, %%r10d;"                       // copy k1 cmp mask to r10d
      "and $1, %%r10d;"                           // isolate 1 bit - only care about zmm17[0]<zmm16[0] @ int32
      "cmp $1, %%r10d;"                           // is rb10==1?
      "je rhsless%=;"                             // rhs less (otherwise lhs less)

      "lhsless%=:;"                               // lhs less branch
      "vpermq %%zmm25, %%zmm30, %%zmm25;"         // right rotate by uint64
      "vpord %%zmm23, %%zmm25, %%zmm25%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm23, %%zmm31, %%zmm23;"         // left rotate by uint64
      "dec %%r8b;"                                // lhscount -= 1 
      "jmp bottomloop1%=;"                        // see if more work
      
      "rhsless%=:;"                               // rhs branch
      "vpermq %%zmm25, %%zmm30, %%zmm25;"         // right rotate by uint64
      "vpord %%zmm24, %%zmm25, %%zmm25%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm24, %%zmm31, %%zmm24;"         // left rotate by uint64
      "dec %%r9b;"                                // rhscount -= 1 
      
      "bottomloop1%=:;"                           // book keeping on counts/dst
      "inc %%r11;"                                // dst count += 1
      "cmp $8, %%r11;"                            // is it 8? if yes, zmm25 full
      "jne bottomloop2%=;"                        // not full
      "vmovdqa64 %%zmm25, %%zmm26;"               // copy to zmm26
      "vpxorq %%zmm25, %%zmm25, %%zmm25;"         // re-zero zmm25 (merge result goes here)

      "bottomloop2%=:;"                           // book keeping on counts/dst
      "cmp $0, %%r8b;"                            // compare lhscount r8b to 0
      "setg %%r10b;"                              // set r10b to hold GT flag from cmp 
      "cmp $0, %%r9b;"                            // compare rhscount r9b to 0
      "setg %%r11b;"                              // set r10b to hold GT flag from cmp 
      "shl $1, %%r11b;"                           // move GT flag over one bit
      "or %%r11b, %%r10b;"                        // combine GT flags
      "cmp $3, %%r10b;"                           // is r10b 3?
      "je loop1%=;"                               // if yes, more loop1 work
      "cmp $0, %%r8b;"                            // was lhs count 0?
      "je rhstailloop%=;"                         // do rhs work

      "lhstailloop%=:;"                           // lhs tail work
      "vpermq %%zmm25, %%zmm30, %%zmm25;"         // right rotate by uint64
      "vpord %%zmm23, %%zmm25, %%zmm25%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm23, %%zmm31, %%zmm23;"         // left rotate by uint64

      "lhstailloop1%=:;"                          // cont lhs tail work
      "inc %%r11;"                                // dst count += 1
      "cmp $8, %%r11;"                            // is it 8? if yes, zmm25 full
      "jne lhstailloop2%=;"                       // not full
      "vmovdqa64 %%zmm25, %%zmm26;"               // copy to zmm26
      "vpxorq %%zmm25, %%zmm25, %%zmm25;"         // re-zero zmm25 (merge result goes here)

      "lhstailloop2%=:;"                          // cont lhs tail work
      "sub $1, %%r8b;"                            // lhscount -= 1 
      "jnz lhstailloop%=;"                        // do more lhs tail work
      "jmp done%=;"                               // all done

      "rhstailloop%=:;"                           // rhs tail work
      "vpermq %%zmm25, %%zmm30, %%zmm25;"         // right rotate by uint64
      "vpord %%zmm24, %%zmm25, %%zmm24%{%%k2%};"  // OR/combine lowest two int32s into zmm18
      "vpermq %%zmm24, %%zmm31, %%zmm24;"         // left rotate by uint64

      "rhstailloop1%=:;"                          // cont rhs tail work
      "inc %%r11;"                                // dst count += 1
      "cmp $8, %%r11;"                            // is it 8? if yes, zmm25 full
      "jne rhstailloop2%=;"                       // not full
      "vmovdqa64 %%zmm25, %%zmm26;"               // copy to zmm26
      "vpxorq %%zmm25, %%zmm25, %%zmm25;"         // re-zero zmm25 (merge result goes here)

      "rhstailloop2%=:;"                          // cont rhs tail work
      "sub $1, %%r9b;"                            // rhscount -= 1 
      "jnz rhstailloop%=;"                        // do more rhs tail work
      
      "done%=:;"                                  // all done
      :
      :
      :
  );
}
void sort() {
  simd_init();
  // merge d0,d1 output 4 elems into zmm18
  merge4_zmm18(data0+0, data0+8);
  asm("vmovdqa64 %%zmm18, %%zmm19;" :::);
  // merge d2,d3 output 4 elems into zmm18
  merge4_zmm18(data0+16, data0+24);
  asm("vmovdqa64 %%zmm18, %%zmm20;" :::);
  // merge d4,d5 output 4 elems into zmm18
  merge4_zmm18(data0+32, data0+40);
  asm("vmovdqa64 %%zmm18, %%zmm21;" :::);
  // merge d5,d6 output 4 elems into zmm18
  merge4_zmm18(data0+48, data0+56);
  asm("vmovdqa64 %%zmm18, %%zmm22;" :::);

  // merge zmm19,20 output 8 elems into zmm23
  merge8_zmm19_20();
  // merge zmm21,22 output 8 elems into zmm24
  merge8_zmm21_22();

  // merge zmm23,24 output 16 elems into zmm25,zmm26
  merge16();

#ifndef NDEBUG
  printf("Sorted:\n");
  printf("-------------------------------------------\n");
  for (int i=0; i<CAP*8; ++i) {
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
  for (int i1=1; i1<8; ++i1) {
  for (int i2=2; i2<8; ++i2) {
  for (int i3=3; i3<8; ++i3) {
  for (int i4=4; i4<8; ++i4) {
  for (int i5=5; i5<8; ++i5) {
  for (int i6=6; i6<8; ++i6) {
  for (int i7=7; i7<8; ++i7) {
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
