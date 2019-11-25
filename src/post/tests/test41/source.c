/* Simulating the artificial main to compare these results with test42.c */

#include <stdio.h>


int foo(int* err, int number) {
  *err = -5;

  if (number > 0) 
    printf("blabla...\n");
  else
    *err = 0; /* overwrites error */

  return *err;
}

void foo1() {
  int err = -5;
  err = 0;  /* overwrites error */
  return;
}


int main() {
  while(1) {
    int number;
    int* arg1;
    int arg2;

    switch(number) {
    case 0:
      foo(arg1, arg2);
      break;
    case 1:
      foo1();
      break;
      default:
      // no calls
      break;
    }
  }

  return 0;
}
