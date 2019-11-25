
int foo() {
  int number;
  int error = -5;
  int rc = -6;

  if (number < 0) {
    return error;
  }

  if (number < 5) {
    return rc;
  }

  if (number  < 10) {
    return 3;
  }

  return 0;
}


int main() {
  foo();
  return 0;
}
