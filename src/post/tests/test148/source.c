// Testing IS_ERR, transformation OFF

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


int main() {
  long err = -5;
  void* errp = (void*)err; // manual error-transformation, but errp not td
  long rc = 0;

  if (IS_ERR(errp)) {
    errp = 0;
  }
  else {
    errp = 0;
  }

  return 0; // err is out of scope
}

// test149 with transformation ON

