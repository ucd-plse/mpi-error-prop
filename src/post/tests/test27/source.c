#include <stdarg.h>
#include <stdio.h>

void foo(int* b, ...) {

  va_list args;
  va_start(args, b);
  int x = va_arg(args, int);
  va_end(args);

  printf("The value is %d\n", x);
  *b = -5;
  return;
}

int main() {
  int x = 0;
  foo(&x, 5);
  x = 6; /*overwriting error*/
  return 0;
}



