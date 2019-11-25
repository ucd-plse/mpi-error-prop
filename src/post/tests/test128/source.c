// Testing error-handling pattern #5 (not only 'retry' as a name, copied from test111

static void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {

  int rc;
  int error = -5;

 retryX:
  rc = (int) ERR_PTR(error); // OK to overwrite on a retry
  // rc does not contain an error anymore, ERR_PTR

  // sometime later a different error is assigned
  rc = -6;
  
  if (rc == -6) {
    // clearing rc
    goto retryX;
  }

  return 0; // rc
}
