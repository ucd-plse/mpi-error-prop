
struct request {
  int result;
  int status;
};

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {
  struct request **r;
  r = ERR_PTR(-5);
  *r = 0;
  return 0;
}
