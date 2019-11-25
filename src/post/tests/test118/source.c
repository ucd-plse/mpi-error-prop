int main() {
  int error = -5;

  if (error == -5) {
    // clear error
    error = 0;  // OK to overwrite
  }

  error = -6; // overwrite

  return 0; // error is out of scope
}
