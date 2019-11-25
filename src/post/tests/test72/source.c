
int main() {
  int error = -5;
  int error2 = 0;

  if (error < 0) {
    error = 0;  // overwriting error!
  }
  else {
    error = 0; // this is ok, error was >= 0
    error2 = -5;
    error = -5;
  }
  error = 0; //overwriting!
  error2 = 0; //overwriting!

  return 0;
}

