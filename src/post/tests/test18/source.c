/*Tests that incorrect merge function gives produces results*/

void foo() {
  int x = 0;
  x = -5;
  return;
}

int main() {
  foo();
  foo();
  return 0;
}



