
static inline void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {
  int err = -30;
  void* v = ERR_PTR(err);
  v = (void *) -5; // overwrite v
  return 0; // copy: v and err out of scope, transfer: v out of scope
}

// original test: 84
