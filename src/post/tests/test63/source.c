
int main() {

  int number;
  int error;
  int var = -5;

  if (number > 100) {
    error = -5;
  }
  else {
    error = -30;
  }
  error = var; /* should not be OK bc error might contain -30*/


  return 0;
}



