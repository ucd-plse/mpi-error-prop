
#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)
#define NULL ((void*)0)


struct request {
  int result;
  int status;
};

struct nested_request {
  struct request req;
};


static inline long PTR_ERR(const void *ptr) {
  return (long) ptr;
}

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}


struct nested_request* foo(void *a, int b) {

  return ERR_PTR(-7);
}

int bar() {
  return -10;
}


int main() {
  struct nested_request *nr = ERR_PTR(-6);
  int error = -8;

  // input errors to foo: -6 and -8
  foo(nr, error);
  // exit errors -6 and -7

  int result = bar();

  return 0;
}
