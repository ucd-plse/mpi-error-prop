
void foo(int *p) {
  if (*p == -5)  {}
  return;
}


int main() {
  int y = -5;
  foo(&y);
  y = 7; /*This is OK as 'y' was checked in conditional expr through pointer p*/
  return 0;
}
