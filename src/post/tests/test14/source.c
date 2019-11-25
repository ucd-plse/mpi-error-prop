

int a;

void foo() {
  int x = 0;
  a = -5;
}

int main() {
  int y;
  a = 5;
  y = 7;
  foo();
  y = a;
  y = 6;

  return 0;
}


