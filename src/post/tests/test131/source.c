// Testing error-handling pattern #4

int foo() {
  return 0;
}

int main() {
  int err1 = -5;
  int err2 = -6;
  int err3 = -7;

  if (err1) {
    err1 = foo() ; // OK to overwrite
  }

  if (err2 == 0) {

  }
  else {
    err2 = foo(); // overwriting (removed pattern)
  }

  if (!err3) {
    err3 = foo(); // no overwrite
  }
  else {
    err3 = foo(); // OK to overwrite
  }
  
  return 0;
}
