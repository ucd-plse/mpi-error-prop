#include <stdio.h>

int main() {
  int x = -5;

  int *y = &x;
  int *z = y;
  *y = 6;     /*Overwriting error*/
  return 0;
}
