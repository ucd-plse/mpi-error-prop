int x;

void foo() {
  int number;
  if (number) {
    x = -5; // overwrite global
  }
  else {
    x = -6;
  }
}


void foo1() {
  int x;
  int number;
  if (number) {
    x = -5;
  }
  else {
    x = -6;
  }
} // out of scope


int main() {
  foo();
  foo();
  foo1();
  foo1();
  return 0;
}
