int foo() {
  int ret = -5;
  return ret;
}

int main() {
  int rc = foo();
  int rc2 = foo(); // no overwrite of foo$return
  return 0; 
}
