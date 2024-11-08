#!/usr/bin/python3.12


CODE='''
void merge4() {
  asm("mov $4, %%r8;"                             // lhs count
      "mov $4, %%r9;"                             // rhs count
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
      :
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

# merge 19+20 -> 23 
replace(["void merge4",          "%zmm18", "%zmm16", "%zmm17"],
        ["void merge8_zmm19_20", "%zmm23", "%zmm19", "%zmm20"])
# merge 21+22 -> 24
replace(["void merge4",          "%zmm18", "%zmm16", "%zmm17"],
        ["void merge8_zmm21_22", "%zmm24", "%zmm21", "%zmm22"])
