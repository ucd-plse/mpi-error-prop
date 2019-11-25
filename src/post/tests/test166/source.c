
#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}

void panic();

int main() {

  int *err  = ERR_PTR(-5);
  int *err2 = ERR_PTR(-6);
  int *err3 = ERR_PTR(-7);
  
  if (IS_ERR(err)) {
    *err3 = 0;
    panic();
  }

  *err  = 0; // never executed if err contains an error, no dereference
  *err2 = 0;
  
  return 0;
}
