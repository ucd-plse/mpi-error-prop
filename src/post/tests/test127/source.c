// Testing error-handling pattern #4

int main() {

  int err = -5;
  int rc = -5;

  if (err) {
    if (rc) {
      err = 0; // no overwrite, err is checked
      rc = 0; // no overwrite, rc is checked
    }
    else {
      err = 0; // no overwrite, err is checked
      rc = 0; // no overwrite, no error in this branch
    }
  }
  else {
    err = 0; // no overwrite, no error in this branch
    rc = 0; // overwrite
  }
  
  return 0;
}
