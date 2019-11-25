struct results {
  int a;
  int b;
};

struct request {
  struct results* result;
  int status;
};

int main() {
  struct request r;
  r.result->a = 10;
  return 0;
}
