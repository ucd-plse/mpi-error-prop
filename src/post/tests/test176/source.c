
#define MAX_ERRNO       4095
//#define unlikely(x)    __builtin_expect(!!(x), 0)
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

struct request {
  int result;
  int status;
};

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}

int main() {
  struct request *r = ERR_PTR(-5);
  struct request *s = ERR_PTR(-6);

  if (unlikely(IS_ERR(r))) {
    int temp = -5;
    temp = r->result; // dereferencing r
    temp = r->result; // no longer reported
    r->result = 0; // no longer reported
    s->result = r->result; // dereferencing s
  }
  else {
    r->result = 0; // dereference is OK
  }

  r = ERR_PTR(-5);
  int x;

  if (r->result > 0) { // dereference reported
    x = r->result; // should not be reported
  }

  x = r->result; // should not be reported
  
  return 0;
}
