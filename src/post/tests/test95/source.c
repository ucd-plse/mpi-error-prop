
int main() {
  int a;
  int err = -5;
  err = a; /*overwriting with an uninitialized variable*/
  return 0;  /*we are currently not reporting out of scope for globals*/
}
