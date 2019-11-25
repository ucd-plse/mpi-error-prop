int main() {
  int error = -5;

  if (error == -5) {
    // clear error
    return 0;  // error is not out of scope
  }

  error = -6; // overwrite

  return 0; // error is out of scope
}
