
#define MAX_ERRNO       4095
#define unlikely(x) (!!(x))
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)
#define NULL ((void*)0)


struct request {
  int result;
  int status;
};

struct nested_request {
  struct request req;
};


static inline long PTR_ERR(const void *ptr) {
  return (long) ptr;
}

static inline void *ERR_PTR(long error) {
  return (void *) error;
}

static inline long IS_ERR(const void *ptr) {
  return IS_ERR_VALUE((unsigned long)ptr);
}


struct nested_request* foo() {
  int number;
  if (number) {
    return ERR_PTR(-7);
  }
  else {
    return NULL;
  }
}

int main() {
  struct nested_request *nr = ERR_PTR(-6);
  int error = -7;
  

  if (IS_ERR(nr)) {
    nr = NULL; // ok to overwrite!
  }

  if (error) {
    error = 0; // ok to overwrite!
  }

  int num = 3;
  while(num) {
    nr = foo(); // of to overwrite!
    if (IS_ERR(nr)) {
      continue;
    }
    num--;
  }

  nr = foo();
  while(unlikely(!nr)) {
    nr = 0; // ok to overwrite!
  }

  int err = -5;
  while(unlikely(!err)) {
    err = 0; // ok to overwrite!
  }

  nr = 0; // overwrite

 out:
  nr = foo(); // ok to overwrite
  
  if (nr == ERR_PTR(-5)) {
    goto out;
  }


  nr = foo(); // overwrite
  if (!IS_ERR(nr)) {
    
  }
  else {
    nr = 0; // ok to overwrite
  }

  return 0;
}
