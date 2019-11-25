int a;

int foo(int* b) {
  *b = -5; /*overwriting an error with an error*/
  return 0;
}

int main() {
  int x = -5;
  int y;

  y = foo(&x);
  y = 5;
  return 0; /*x is out of scope*/
}

// this test with error transformation enabled: test159


