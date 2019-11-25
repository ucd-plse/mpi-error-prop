
int foo() {
  int rc = -5;
  if (rc != 0) {
    return rc;
  }
  return 0; 
}

int main() {
  foo(); // unsaved error
  return 0;
}
