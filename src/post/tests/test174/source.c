struct request {
  int result;
  int status;
};

void unknown(int);

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

void foo(int result) {
  return;
}

int bar(struct request *r) {
  return r->status; // no longer reported
}

int main() {
  struct request *r = ERR_PTR(-5);
  foo(r->result); // dereference of error code
  bar(r); // we clear variable
  unknown(r->status); // no longer reported
  return 0;
}
