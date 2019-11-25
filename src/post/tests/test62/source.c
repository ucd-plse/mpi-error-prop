
int main() {

  int number;
  int error;

  if (number > 100) {
    error = -5;
  }
  else {
    error = -30;
  }
  error = -5; /* should not be OK bc error might contain -30*/


  return 0;
}



