
int foo1() {
  return -5;
}

void foo() {
  int x = -5;
  return; /*x out of scope*/
}


int main() {
  int y = -5;
  foo();
  foo1(); /*unsaved error*/
  return 0;   /*y out of scope*/
}
