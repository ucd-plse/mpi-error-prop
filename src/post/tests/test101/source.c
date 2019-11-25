/* Testing positive error codes */

int main() {
  int err = 5;

  if (err) {
    err = 0; // before overwrite, now OK since the error is 'acknowledged'
  }
  else {
    err = 6; // ok
  }

  if (!err) {
    err = 0; // ok
  }
  else {
    err = 4; // overwrite
  }

  return 0;
}
