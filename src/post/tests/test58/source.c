
int foo1() {
  return 0;
}

void foo() {
  int x = -5;
  return; /*x out of scope*/
}


int main() {
  int y = -5;
  foo();
  foo1();
  return 0;   /*y out of scope*/
}
