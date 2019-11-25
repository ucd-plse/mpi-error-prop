#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

int *g;

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

  // copy: v, err, err2, x, and y.DEREF have errors at this point
  // transfer: v, err2, and y.DEREF have errors at this point

  if (IS_ERR(err2)) {
    err2 = 0; // overwrite err2
  }


  if (IS_ERR(v)) {
    *v = 0; // ok? double check when revisiting overwrites
    v = 0;
  }
  else {
    *v = -7; 
    v = 0; // v.DEREF is overwritten
  }
  
  // copy: err, x, and y.DEREF out of scope
  // transfer: y.DEREF out of scope

  return 0; 
}
