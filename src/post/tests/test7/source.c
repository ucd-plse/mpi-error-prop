

int main() {
  int x = 0;
  int y = -5;
  int z = 0;

  if (z == x) { //changed from y == x to z == x, otherwise test would fail with new if expr
     x = -5;
  }
  else {
     x = 6;
  }
  y = 7; /*Overwriting error */
  z = x;
  x = 6;  /*Overwrite error for copy*/
  y = 0;
  z = 10; /*Overwrite error*/
  y = 8;
  return 0;
}
