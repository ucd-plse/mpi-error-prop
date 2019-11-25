void foo1(char a, int b) {
  int c = 6;
}

void foo(int* b) {
  *b = -5;
  return;
}

int mainA() {
  int x = 0;
  foo(&x);
  x = 6;
  return 0;
}



