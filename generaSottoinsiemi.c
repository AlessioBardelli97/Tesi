#include <stdio.h>
#include <stdlib.h>

void print(int* a, int n) {
  int i;
  for (i=0; i<n; i++) {
    printf("%d ",a[i]);
  }
  printf("\n");
}
 
void generabk(int* b, int n, int i, int k) {
  int j;
  if (k == 0) print(b, n);
  else if (k == n-i) {
     for (j = i; j < n; j++) b[j] = 1;
     print(b, n);
     for (j = i; j < n; j++) b[j] = 0;
  } else {	
    b[i] = 0;
    generabk(b, n, i+1, k);
    b[i] = 1;
    generabk(b, n, i+1, k-1);
    b[i] = 0;
  }
}

int main() {
  
  int b[5] = {0,0,0,0,0}, i;

  for (i=0;i<=5;i++) {
    printf("k=%d\n", i);
    generabk(b,5,0,i);
    printf("\n");
  }

  return 0;
}
