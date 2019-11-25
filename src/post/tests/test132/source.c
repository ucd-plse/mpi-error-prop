void foo() {}

int main() {

  int number;
  if (number) {
    foo();
  }
  else {
    foo();
    foo();
  }

  if (number) {
    number =-5;
  }
  return 0;
}
