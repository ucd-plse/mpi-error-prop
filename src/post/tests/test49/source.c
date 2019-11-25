/* A CIL generated test with no successors is generated for this program, however
   that statement is unreachable */

#include <stdio.h>


int foo() {

  int number;

  switch(number) {
  case 1:
    return 1;
  case 2:
    return -5;
  default:
    return 0;
  }
}

int main() {
  int err;

  err = foo();
  printf("%d", err);
  err = 0; // overwrite
  
  return 0;
}



