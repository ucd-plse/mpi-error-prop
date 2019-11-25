
int foo() {
  int number;
  int rc = 0;

  if (number < 0) {
    rc = -5;
    return rc;
  }

  if (number < 5) {
    rc = -6;
    return rc;
  }
  return rc;
}


int main() {
  foo();
  return 0;
}
