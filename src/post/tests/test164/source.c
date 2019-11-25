
static inline void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {

  int *v = ERR_PTR(-5);
  int x = *v;  // potential segfault
  *v = 0; // no longer reported  
  
  return 0;
}
