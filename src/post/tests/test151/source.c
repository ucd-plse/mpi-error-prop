// Testing IS_ERR, transformation ON

#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

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
    error = 0; // overwrite
  }
  error = (void *) -6;

  return; // copy and transfer: err
}


int main() {
  int err = -5;
  void* errp = (void*)err; // manual error-transformation
  foo(errp);
  
  return 0; // copy: err, errp
}

// test150 with transformation OFF
