
int foo() {
  
  int number;
  int error1 = 0;
  int error2 = 0;

  if (number) {
    error1 = -5;
  }
  else {
    error1 = -6;
  }

  if (number) {
    error2 = -7;
  }
  else {
    error2 = -8;
  }

  error1 = error2; // -5 or -6 is overwritten by -7 or -8

  return error1;
}

int main() {

  int number;
  int error = 0;

  if (number) {
    error = -9;
  }
  else {
    error = -10;
  }

  error = foo();

  error = -5;

  return 0;
}
