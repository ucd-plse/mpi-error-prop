
int foo() {
  return 0;
}

int main() {
  int err = -5;
  err = foo(); // overwrite
  return 0;
}
