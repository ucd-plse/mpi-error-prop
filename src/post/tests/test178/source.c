
#define MAX_ERRNO       4095
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
  struct request p = {0, 1};
  struct request *r = &p;
  struct request *s = ERR_PTR(-6);
  int number;

  if (number) {
    r = ERR_PTR(-5);
  }
  
  int temp;
  temp = r->result; // dereferencing r, may contain an error
  temp = s->result; // dereferencing s, must contain an error

  return 0;
}
