

long foo() {
  int error = -30;
  return error; //the error should be propagated
}

int main() {
  int v;
  v = foo(); 
  v = -5; //attempting to overwrite error -30 with error -5!!
  return 0; //v is out of scope without being checked
}

