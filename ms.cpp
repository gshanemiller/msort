#include <stdio.h>

int d0[3] = {1, 2, 3};
int d1[3] = {0, 0, 0};
int d2[3] = {10, 10, 10};
int d3[3] = {9,9,10};
int d4[3] = {1,1,1};
int d5[3] = {3,3,3};

int c[6];

int *data[6] = {d0,d1,d2,d3,d4,d5};

void sort(int *a, int *b, int *c) {
  int i=0;
  int j=0;
  int writeIdx = 0;

  while (i<3 && j<3) { 
    if (b[j]<a[i]) {
      c[writeIdx++] = b[j++]; 
    } else {
      c[writeIdx++] = a[i++]; 
    }
  }

  while (i<3) {
    c[writeIdx++] = a[i++]; 
  }
  while (j<3) {
    c[writeIdx++] = b[j++]; 
  }

  for (int k=1; k<6; ++k) {
    if (c[k]<c[k-1]) {
      printf("error\n");
    }
  }
}

int main() {
  for (int i=0; i<6; ++i) {
    for (int j=0; j<6; ++j) {
      for (int k=0; k<6; ++k) {
        c[k]=-1;
      }
      sort(data[i], data[j], c);
    }
  }
  return 0;
}
