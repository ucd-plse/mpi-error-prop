
int main() {

  int number;
  int error;

  if (number > 50) {
    error = -5;
    if (number > 100) {
      error = 0; /* overwrites error */
    }
    else {
      if (number > 75) {
	error = 0; /* overwrites error */
      }
      else {
	error = 0; /* overwrites error */
      }
      error = 1; /* this should be OK, the error is gone by now */
    }
  } 
  else {
    error = 1; /* this should be OK, error did not contain an error if this branch was taken */
  }

  return 0;
}



