

struct request {
  int result;
  int status;
};

void foo(struct request *req, int result) {
  req->result = 0;
  req->status = 0;
}

int main() {
  int err = -5;
  struct request *r;
  
  foo(r, err);
  
  return 0;
}
