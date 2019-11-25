void foo(int error) {
  return; // out of scope error but trace is truncated
}

int main() {
  int x = -5; // this should be the start of the trace
  foo(x);
  return 0;
}

