typedef int int32_t;
typedef int32_t I32;
typedef I32 NvdsBoxNr;

int foo() {
  return -5;
}

int foo1(NvdsBoxNr b) {
  return 0;
}

int main() {
  NvdsBoxNr box2, merge, res;

  box2 = foo();

  if ((merge != 0) && (box2 >= 0)) {
    res = foo1(box2);
  }

  return 0;
}
