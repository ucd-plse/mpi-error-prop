#define unlikely(x)    __builtin_expect(!!(x), 0)

void foo() {
  return;
}

int main() {
  int error = -5;
  
  if (error)
    goto exit;

  foo();
  foo();
  foo();

 exit:
  if (unlikely(error))
    error = 0;

  return 0;
}
