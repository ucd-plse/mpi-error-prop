#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)


struct request {
  int result;
  int status;
};

struct nested_request {
  struct request *req;
  int value;
};


static inline void *ERR_PTR(long error) {
  return (void *) error;
}


int bar(int x) {
  return x;
}


struct request *foo(struct nested_request* pnr) {
  return pnr->req; // dereference
}


void release(struct nested_request* pnr) {

  if (bar(-5)) {
    int x = 0;
  }
  else {
    pnr = 0;
  }

  foo(pnr);
}





int main() {

  struct nested_request *pnr = ERR_PTR(-6);  
  release(pnr);

  return 0;
}
