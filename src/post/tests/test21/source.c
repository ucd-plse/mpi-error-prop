int a;

int foo(int a, int* b, char c) {
  *b = -5;
  return a;
}

int main() {
  int x = 0;
  int y;

  y = foo(5, &x, 'c');
  x = 6; /*Overwriting error*/
  y = 5;
  return 0;
}

// this with error transformation enabled: test158



