
/* Testing positive error codes */
int main() {
  int error = 5;

  if (error < 0) {
    error = 0; // OK
  }
  else {
    error = 6; //overwriting
  }
  
  if (error <= 0) {
    error = 0; // OK
  }
  else {
    error = 5; // overwriting
  }

  if (0 > error) {
    error = 0; // OK
  }
  else {
    error = 6; // overwriting
  }

  if (0 >= error) {
    error = 0; // OK
  }
  else {
    error = 5; // overwriting
  }

  return 0;
}
