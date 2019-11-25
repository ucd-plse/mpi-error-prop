int a;

int foo(int a, int* b, char c) {
  a = 5;
  return a;
}

int main() {
  int x = 0;
  int y;

  y = foo(5, &x, 'c');
  x = 5;
  return 0;
}
