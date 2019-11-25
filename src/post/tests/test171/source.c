struct request {
  int result;
  int status;
};

extern struct request **r;

void foo(struct request *p) {
  return;
}

void foo1(void* t) {
  return;
}

int main() {
  int error = -5;
  foo(*r);
  error = -6;
  foo1(*r);
  error = 0;
  return 0;
}
