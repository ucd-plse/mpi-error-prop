void* ERR_PTR(long err) {
  return (void *)err;
}

void* foo() {
  return ERR_PTR(-6);
}

int main() {
  int* rc = ERR_PTR(-5);
  rc = foo();
  return 0;
}
