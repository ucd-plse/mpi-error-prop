//test156 but error transformation is disabled
int *g;

#define MAX_ERRNO       4095
#define unlikely(x)    __builtin_expect(!!(x), 0)
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}

int main() {
  // testing old v* = error_constant transfer function
  // error transformation is disabled, v <- -5
  int *v;
  *v = -5;

  // testing additional transfer functions when error transformation is disabled
  v = (void *) -6; // *v is overwritten, but the assignment of error to pointer variable not recorded

  int *err;
  err = (void *) -5; // do not allow to hold errors

  int *err2;
  err2 = err; // nothing happens

  // testing old v1 = &v2
  int x = -5;
  int *y;
  y = &x;

  // copy: v, err, err2, x, and y have errors at this point
  // transfer: v, err2, and y have errors at this point

  if (IS_ERR(err2)) {
    err2 = 0;
  }
 

  if (IS_ERR(v)) {
    *v = 0;
    v = 0; // no overwrite because pointers can't hold errors
  }
  else {
    v = 0; // no overwrite because pointers can't hold errors
    *v = 0;
  }
  
  // copy: x, and *y out of scope
  // transfer: y out of scope

  return 0;
}
