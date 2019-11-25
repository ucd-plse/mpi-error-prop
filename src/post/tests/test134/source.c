int x;

void foo1() {
  x = -5; // OK to overwrite with same error
  int number;
  if (number) {
    foo1();
  }
}

void foo() {
  int x = -5;
} // out of scope

int main() {
  int x = -5;
  foo();
  foo1();
  x = 6; // overwriting
  return 0;
}
