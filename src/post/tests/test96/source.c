

struct request {
  int result;
  int status;
};

int foo() {
  return -5;
}

void foo1(int);

int main() {
  int err = -5;
  struct request *r;
  
  r->result = foo();

  int a;
  foo1(a & 1);

  err = 0; /*err is overwritten*/
  
  return 0; 
}
