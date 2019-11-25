// Test to make sure that nested error handlers information is available

int foo() {
  return -5;
}

void callA();
void callB();

int main() {
  int err = foo();

  if (err) {
    callA();
    if (err) {
      callB();
    }
  }
}
