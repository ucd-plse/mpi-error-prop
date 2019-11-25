int *g;

int IS_ERR_VALUE(unsigned long);

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}


int main() {
  // testing old v* = error_constant transfer function
  // since we performed replacements, this is now assigned to v.DEREF
  int *v;
  *v = -5;

  // testing additional transfer functions when error transformation is ON
  v = (void *) -5; // overwriting v.DEREF

  int *err;
  err = (void *) -5;

  int *err2;
  err2 = err;

  // testing old v1 = &v2
  int x = -5;
  int *y;
  y = &x;

  // out of scope in copy mode: v, err, err2, x, y.DEREF
  // out of scope in transfer mode: v, err2, y.DEREF
  return 0; 
}
