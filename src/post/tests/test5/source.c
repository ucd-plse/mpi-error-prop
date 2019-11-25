#include <stdio.h>

int main() {
  int x = 0;
  int y = 0;
  int z = 0;

  if (y == x) {
     x = -5;
  }
  else {
     x = 6;
  }
  y = 7;
  z = x;
  x = 6; /*Overwrite for copy, not for transfer*/
  y = 8;
  return 0; /*z is out of scope*/
}
