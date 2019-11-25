// Testing IS_ERR, transformation OFF

#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)


void printk();


static inline long PTR_ERR(const void *ptr) {
  return (long) ptr;
}


static inline void *ERR_PTR(long error) {
  return (void *) error;
}


static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}


void foo(void* error) {
  if (IS_ERR(error)) {
    error = 0;
  }
  error = (void *) -6;

  return;
}


int main() {
  int err = -5;
  void* errp = (void*)err; // manual error-transformation, but errp is not td
  foo(errp);
  printk();

  return 0; // err is out of scope 
}

// test151 with transformation ON
