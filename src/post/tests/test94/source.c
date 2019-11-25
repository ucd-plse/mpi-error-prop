

struct request {
  int result;
  int status;
};

void foo(struct request *req, int result) {
  req->result = result;
  req->status = 0;

  result = req->result; /*overwriting result*/
}

int main() {
  int err = -5;
  struct request *r;
  
  foo(r, err);
  
  return 0; /*copy: err out of scope, transfer: no error*/
}
