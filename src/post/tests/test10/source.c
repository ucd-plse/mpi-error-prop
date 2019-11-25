#include <stdio.h>

int main() {
  int x = -5;

  int *y = &x;
  int *z = y;
  *y = 6;

  int **a = &y;
  **a = 7;

  int e =4;
  y = &e;
  return 0;
}
