int foo() {
  int ret = -5;
  return ret;
}

int foo1() {
  int err = foo();
  return err;
}


int foo2() {
  int err2 = foo1();
  return err2;
}


int main() {
  foo2(); // non-tentative error is not saved
  return 0; 
}
