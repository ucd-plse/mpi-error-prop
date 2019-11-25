
struct request {
  int result;
  int status;
};

static inline void *ERR_PTR(long error) {
  return (void *) error;
}


struct request *get_request() {
  return ERR_PTR(-7);
}


int main() {
  struct request r = {-5, 0};
  struct request *pr = &r;

  pr->result = -6;
  pr = get_request();
  pr = (void *) -5; // overwrite
  return 0; // pr out of scope
}
