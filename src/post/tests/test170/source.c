struct request {
  int result;
  int status;
};

extern struct request *r;

void foo(struct request *p) {
  return;
}

int main() {
  int error = -5;
  foo(r);
  error = 0;
  return 0;
}
