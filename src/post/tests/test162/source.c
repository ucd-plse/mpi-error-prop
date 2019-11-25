
struct dentry {
  unsigned int dflags;
  unsigned int d_time;
  int error;
} dentry;

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

void foo(struct dentry *e) {
  return; /*e is out of scope because pointer passed by value*/
}

int main() {
  struct dentry *d = ERR_PTR(-5);
  foo(d);
  return 0;
}
