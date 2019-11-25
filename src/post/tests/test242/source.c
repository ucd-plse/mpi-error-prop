int foo() {
  return -5;
}

int bad() {
  return 0;
}

void A();
void B();
void C();
void D();

int main() {
  int err = foo();
  int x = bad();

  if (err) {
    A();
  }

  if (err) {
    B();
  }

  if (err && x) {
    C();
  } 

  if (x) {
    D();
  }
}
