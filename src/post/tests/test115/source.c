void cERROR(const char *fmt, ...) {}

int foo() {
  int rc = -5;
  if (rc != 0) {
    cERROR("Unable to open file to get ACL");
    return 0; /*rc does not go out of scope anymore */
  }
  return rc;
}

int main() {
  int err = foo();
  return 0;
}
