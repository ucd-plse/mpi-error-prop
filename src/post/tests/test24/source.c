
void foo(int* b) {
  *b = -5;
  return;
}

int main() {
  int x = 0;
  foo(&x);
  x = 6; /*overwriting error*/
  return 0;
}



