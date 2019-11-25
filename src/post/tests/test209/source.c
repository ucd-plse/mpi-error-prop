int foo() {
  return -5;	
}

int main() {
  int rc = foo();
  int x = 0;
  x = rc;

  if (x < 0) {
    return -1;
  } else {
    return 0;
  }
}
