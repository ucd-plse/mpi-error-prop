// Testing PTR_ERR, transformation ON

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


int foo(void* error) {
  int rc = PTR_ERR(error);
  return rc; // copy: error
}


int main() {
  int err = -5;
  void* errp = (void*)err; // manual error-transformation
  int err2 = foo(errp);
  
  return 0; // copy: err, errp, err2; transfer: err2
}

// test152 with transformation OFF
