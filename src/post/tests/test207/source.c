
int foo() {
  return -5;
}

int main() {
  if (foo() < 0) {
    return -1;
  } else {
    return 0;
  }
}
