/* Generating the artificial main to compare these results with test41.c */

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

