#include <stdio.h>



int* id(int *v) {
  return v;
}


int main() {
  int a = 0;
  int b = 1;
  

  int *x = id(&a);
  int *y = id(&b);

  return 0;
}
