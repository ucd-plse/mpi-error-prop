
int foo() {
  int retval = -5;

  if (retval) {
    int err = -6;
    return err ? err : retval; // retval out of scope, err out of scope?
  }

  return retval;
}

int main() {

  foo(); // error code is not saved
  return 0;
}
