int foo() {
  int number;
  if (number > 5) {
    return -5;
  }
  else if (number < 3) {
    number = -6;
  }
  else {
    number = -7;
  }
  return number;
}


int main() {
  int x = foo();
  return 0;
}
