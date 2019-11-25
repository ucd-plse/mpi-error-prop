
#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)
#define NORET_TYPE    /**/

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}

static inline long IS_ERR_XXX(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}

NORET_TYPE void panic(const char * fmt, ...) {
}

int main() {

  int *err = 0;  
  if (IS_ERR(err)) {
    //*err = 0;// something
  }
  else {
    *err = 0;
  }

  int *err1 = ERR_PTR(-5);
  if (IS_ERR(err1)) {
    // something
  }
  else {
    *err1 = 0;
  }  

  return 0;
}
