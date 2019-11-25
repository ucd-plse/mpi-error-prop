
#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)


struct request {
  int result;
  int status;
};

void printk(const char *fmt, ...) {}


static inline void *ERR_PTR(long error) {
  return (void *) error;
}


void foo(struct request* t) {
  return;
}


int main() {

  struct request* r = ERR_PTR(-5);
  struct request* s = r + 1; // invalid pointer arithmetic
  r++; // r is cleared
  
  r = ERR_PTR(-6);
  s = r - 1; // invalid pointer arithmetic
  r--; // r is cleared
  
  r = ERR_PTR(-7);
  s = ERR_PTR(-8);
  s = (void *) (s - r); // invalid pointer arithmetic
  
  r = ERR_PTR(-7);
  foo(r + 2); // invalid pointer arithmetic

  printk("INFO: 0x%p 0x%p\n", r + 1); // should not be reported
  s = r + 1; // invalid pointer arithmetic

  printk("INFO: 0x%p 0x%p\n", r); // r should not be cleared (--noerrorhandling)
  s = r + 1; // invalid pointer arithmetic

  struct request t = {3, 4};
  s = r + t.result + sizeof(void *);

  return 0;
}
