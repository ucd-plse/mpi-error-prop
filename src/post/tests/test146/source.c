/* Gotos and overwrite false positives in ext3 and ext4 */

static void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {

  int rc;
  int error = -5;

 retry:
  rc = (int) ERR_PTR(error); // OK to overwrite on a retry
  // rc does not contain an error anymore, ERR_PTR

  // sometime later a different error is assigned
  rc = -6;
  
  if (rc == -6) {
    // clearing rc
    goto retry;
  }

  return 0; // rc is out of scope
}

// Original test: 111
