// Testing function pointers

#include <stdio.h>

void foo(int* b, void (*myFunc)(int, int)) {
  myFunc(0, 3);
  *b = -5;
  return;
}

void foo1(int a, int b) {
  printf("In foo1! a = %d b = %d\n", a, b);
  return;
}

int main() {
  int x = 0;
  foo(&x, &foo1);
  x = 6; /*overwriting error*/
  return 0;
}



