
/* Testing positive error codes */
int main() {
  int err = 5;
  int rc = 6;

  if (err > 0) {
    rc = err; // Overwriting
    err = 0; // copy: overwrite, transfer: ok
  }
  else {
    err = 6; // ok
    rc = 5; // overwriting
  }
  
  if (err >= 0) {
    rc = err; // Overwriting
    err = 0; // copy: overwrite, transfer: ok
  }
  else {
    err = 5; // ok
    rc = 6; // overwrite
  }

  if (0 < rc) {
    err = rc; // Overwriting
    rc = 0; // copy: overwrite, transfer: ok
  }
  else {
    err = 6; // overwrite
    rc = 5; // ok
  }

  if (0 <= rc) {
    err = rc; // Overwriting
    rc = 0; // copy: overwrite, transfer: ok
  }
  else {
    err = 5; // overwrite
    rc = 6; // ok
  }

  return 0;
}
