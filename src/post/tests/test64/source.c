
int main() {

  int number;
  int error;
  int var;

  if (number > 100) {
    error = -5;
    var = -30;
  }
  else {
    error = -30;
    var = -5;
  }

  error = var; /* should not be OK */


  return 0;
}



