int foo1() {
  return -5;
}

int foo2() {
  return -6;
}

void A();

int main() {
  int x, err;
  if (x) {
    err = foo1(); 
  } else {
    err = foo2();
  }

  if (err) {
    A();
  }
}
