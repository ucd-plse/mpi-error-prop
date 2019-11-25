int putError(int* result) {
  *result = -5;
  return 0;
}

int main() {
  int code = 0;
  code = putError(&code);
  return 0;
}
