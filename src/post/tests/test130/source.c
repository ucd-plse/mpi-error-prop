// Testing error-handling pattern #2

void cERROR(int button, const char *fmt) {}

int foo() {
  return 0;
}

int main() {
  int err = -5;
  int rc = -6;

  cERROR(1, ("Unable to open file to get ACL"));
  err = foo(); // OK to overwrite

  cERROR(1, ("Unable to open file to get ACL"));
  rc = 0; // OK to overwrite
  
  return 0;
}
