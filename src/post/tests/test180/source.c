
#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)


struct request {
  int result;
  int status;
};

struct nested_request {
  struct request req;
};


static inline void *ERR_PTR(long error) {
  return (void *) error;
}


static inline void check_request(const struct request* r) {
  int res = r->result;
}

void foo(struct nested_request *nr) {
  check_request(&nr->req); // error-value dereference reported
}

int main() {
  struct nested_request *nr = ERR_PTR(-6);
  foo(nr);
  
  struct request r = nr->req; // should not be reported in transfer mode

  return 0;
}
