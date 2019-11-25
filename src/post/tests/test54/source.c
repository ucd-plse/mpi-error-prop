
void foo() {
  int x = -5;
  return; /*x is out of scope*/
}


int main() {
  int y = -5;
  foo();
  foo();
  return 0; /*y is out of scope*/
}
