
void foo(int* b) {
  *b = -5;
  return;
}

void foo1(int *);

int main() {
  int x = -5;
  foo1(&x); /*The definition for foo1 is not available, so it is treated as a black-box.*/
  x = 6; /*Overwriting error*/
  return 0;
}



