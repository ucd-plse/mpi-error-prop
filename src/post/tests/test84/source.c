
static inline void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {
  int err = -30;
  void* v = ERR_PTR(err); // err cleared, v is not tracked down
  v = (void *) -5;
  return 0;
}

