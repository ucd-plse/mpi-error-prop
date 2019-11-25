
int main() {
  int a = -5;
  int b = -5;
  int c = -5;

  if (a < 0) {
    b = 0; // overwriting error!
    a = 0; // overwriting error!
  }
  else {
    b = 0; // overwriting error!
    a = 0; // OK
    a = -5;
  }
  c = 0; // overwriting error!
  b = 0; // OK
  a = 0; // overwriting error!
  

  return 0;
}

