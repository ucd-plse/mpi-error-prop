
int foo() {
  return -5;
}

int main() {
  int z;
  int y = -5;
  foo();
  z = 4;
  return 0;   //y out of scope, __cil for foo out of scope
}
