int foo1() {
  return -5;
}

int foo2() {
  return -6;
}

void A();
void B();

int main() {
  int err, x;

  if (x) {
    err = foo1();
  } else {
    err = foo2();
  }

  if (err == -5) {
    A();
  } 
}
