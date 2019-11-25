
int main() {

  int number;
  int error;

  if (number > 100) {
    error = -5;
  }
  else {
    if (number > 75) {
      error = -5;
    }
    error = 1; /* might overwrite error */
  }

  return 0;
}



