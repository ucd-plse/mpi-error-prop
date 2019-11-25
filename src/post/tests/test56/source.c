
void foo() {
  int x = -5;
  return; /*x is out of scope, but foo is not called*/
}


int main() {
  int y = -5;
  return 0;   /*y is out of scope*/
}
