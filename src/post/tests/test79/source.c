
int main() {
  int error = -5;

  if (error <= 0) {
    error = 0; // overwriting error!
  }
  else {
    error = 0; // OK
  }
  error = 0; // OK

  return 0;
}

