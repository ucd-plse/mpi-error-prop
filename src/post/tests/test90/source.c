
void foo2(int err) {
  return;
}

void foo1(int err) {
  err = -1; /*err is overwritten*/
} /*err out of scope (-1 is EPERM error)*/


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
