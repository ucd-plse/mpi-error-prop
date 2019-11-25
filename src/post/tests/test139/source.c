

struct dentry {
  unsigned int dflags;
  unsigned int d_time;
  int error;
} dentry;


struct dentry* foo(struct dentry* d) {
  d->error = -5;
  return d;
}


int main() {
  struct dentry d = {0, 0, 0};
  struct dentry *retVal = foo(&d);
  return 0;
}
