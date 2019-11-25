
void foo(unsigned long x) {
  x = 0;
}

int main() {
  int error = -5;
  void* addr = 0;
  foo((unsigned long)addr);
  error = 0;
  return 0;
}
