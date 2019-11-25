
#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)
#define NULL ((void*)0)

static inline long PTR_ERR(const void *ptr) {
  return (long) ptr;
}

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}


void* bar() {
  return ERR_PTR(-6);
}

int foo() {
  int number;
  if (number) {
    return -5;
  }
  else {
    void* err = bar();
    return PTR_ERR(err);
  }
}


int main() {
  int err = foo();
  return 0;
}
