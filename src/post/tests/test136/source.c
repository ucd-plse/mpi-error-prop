#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

int main() {
  int error = -5;
  
  if (likely(error))
    goto out;
  
  error = 0;
  
 out:
  return 0; // error is out of scope
}
