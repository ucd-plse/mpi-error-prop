int foo() {
  return -5;	
}

int main() {
  int rc = foo();

  if (rc < 0) {
    return -1;
  } else {
    return 0;
  }
}
