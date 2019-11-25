/* This test must fail if we do not get rid of the caller's locals before the recursive call to foo */

void foo() {
  int i;
  int err = 0;

  if (i > 0) {
    err = -5;
    foo();
    i--;
  }
  
  return;
}


int main() {

  foo();
  return 0;
}

