/*Tests that incorrect merge function gives incorrect results*/
int a;

void foo() {
  int x = 0;
  a = 6; /*Overwriting error the second time foo is called*/
  return;
}

int main() {
  foo();
  a = -5;
  foo();
  return 0;
}



