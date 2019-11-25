

int foo1_vn() {
  int error = 5; // positive error code
  return -error; // should report 5
}

int foo2() {
  return 6; // positive error code, should report 6
}

int foo3_vn() {
  return -foo2(); // should report 6
}

int main() {
  int err = foo3_vn();
  int err2 = foo1_vn();

  return 0;
}
