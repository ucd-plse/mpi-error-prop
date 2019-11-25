
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
  void* errp = ERR_PTR(err);
  long rc = 0;

  if (!IS_ERR(errp)) {
    errp = 0; // no overwrite
  }
  else {
    rc = PTR_ERR(errp);
    rc = 0; // overwrite
  }

  // errp may contain an error code at this point
  if (!IS_ERR(errp)) {
    errp = 0; // no overwrite
  }
  
  return 0; // copy: err, errp
}

