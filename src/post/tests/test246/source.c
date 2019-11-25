
void foo(int error) {
  return; // out of scope error but trace is truncated
}

int main() {
  foo(-5); // this should be the start of the trace
  return 0;
}
