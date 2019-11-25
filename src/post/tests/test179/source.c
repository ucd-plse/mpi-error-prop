
#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

#define foo(r) ({int number, res; if (number) res = r; number;})


struct request {
  int result;
  int status;
};


static inline void *ERR_PTR(long error) {
  return (void *) error;
}

void bar(struct request* t) {
  int temp = t->result; // not reported, earlier dereference already reported
  return;
}

int main() {
  struct request *s = ERR_PTR(-6);

  int r = foo(s->result);
  bar(s);

  return 0;
}
