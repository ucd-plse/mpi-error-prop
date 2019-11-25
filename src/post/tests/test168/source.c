
struct request {
  int result;
  int status;
};

typedef struct request *request_t;

int filp_close(struct request *r1, request_t r2) {
  return 0;
}

int main() {
  int error = -5;
  struct request* r1;
  struct request* r2;
  filp_close(r1, r2);
  error = 0;
  return 0;
}
