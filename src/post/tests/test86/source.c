
int foo() {
  int err = -5;
  return err;
}

int main() {
  int tentative_error = -5;
  int error = foo();

  tentative_error = 0; // OK
  error = 0; // NOT OK

  return 0;
}

