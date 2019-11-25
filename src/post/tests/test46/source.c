#include <stdio.h>


int foo() {

  int number = -5;
  return number;
}

int main() {
  int err;

  err = foo();
  printf("%d", err);
  err = 0;
  
  return 0;
}



