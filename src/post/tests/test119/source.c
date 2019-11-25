int foo() {
  int error = -5;

  if (error == -5) {
    // clear error
    return 0;  // error is not out of scope (ETRANSF)
  }

  error = -6; // overwrite

  return 0; // error is out of scope
}


int main() {
  foo();
  return 0;
}
