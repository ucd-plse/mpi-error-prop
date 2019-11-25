#include <stdio.h>


int foo() {
  return -5;
}

int main() {
  int err;

  err = foo();
  printf("%d", err);
  err = 0;
  
  return 0;
}



