
int main() {
  int error = -5;

  if (0 <= error) {
    error = 0; // OK
  }
  else {
    error = 0; // overwriting error!
  }
  error = 0; // OK

  return 0;
}

