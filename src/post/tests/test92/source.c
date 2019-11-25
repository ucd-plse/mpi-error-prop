
void foo2(int err) {
  return;
}

void foo1(int err) {
  err = 0; /*err is overwritten*/
}


int main() {
  int err = -5;

  if (err < 0) {
    foo1(err);
  }
  else {
    foo2(err); /*err does not contain an error*/
  }

  return 0;  /*copy: err out of scope, transfer: no error*/
}
