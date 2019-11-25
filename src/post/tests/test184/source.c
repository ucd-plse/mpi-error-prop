
static inline void *ERR_PTR(long error) {
  return (void *) error;
}

struct request {
  int result;
  int status;
};

struct request* t;

int main() {

  int x = -5;
  int *px = &x;
  px = ERR_PTR(-6);

  struct request r = {0, -5};

  struct request *s;
  s->result = -5;
  s = ERR_PTR(-7); // if field sensitive, s.result overwritten

  t->result = -6;
  t = ERR_PTR(-8); // if field sensitive, t.result overwritten (global)

  return 0;
}
