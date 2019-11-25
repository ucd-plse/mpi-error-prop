static inline void *ERR_PTR(long error) {
  return (void *) error;
}

int *foo() {
  int err;
  int x;

  if (x < 0) {
    err = -5;
    goto fail;
  } else {
    err = -6;
  }

fail:
  return ERR_PTR(err);
}
