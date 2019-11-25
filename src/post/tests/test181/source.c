
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


void bar(int *x) {
  *x = -9;
  return;
}

struct request *foo(struct nested_request* pnr) {
  return pnr->req; // dereference
}


int main() {

  struct nested_request *pnr = ERR_PTR(-6);  
  struct request* r = foo(pnr);
  r = (void *) -5; // ok
  int *x = ERR_PTR(-7);
  *x = -8; // dereference, both x and *x may contain errors at some point
           // testing exchange variable assignments to both x and *x
  *x = 0; // clearing it, to check if bar's value is returned through pointer
  bar(x);
  x = ERR_PTR(-10);
  *x = -11; // dereference

  return 0;
}
