int a;

int foo(int* b) {
  *b = -5; /*overwriting an error with an error*/
  return 0;
}

int main() {
  int x = -5;
  int y = 0;

  y = foo(&x);
  x = 6; /*overwriting error again*/
  y = 5;
  return 0;
}



