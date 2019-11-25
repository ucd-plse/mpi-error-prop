
int main() {
  int error = -5;

  if (0 > error) {
    error = 0; // overwriting error!
  }
  else {
    error = 0; // OK
  }
  error = 0; // OK

  return 0;
}

