/* Gotos and out-of-scope false positives in ext3 and ext4 */

int foo() {
  int rc, something;

 retry:
  
  if (something) {
    return 0;  // rc does not go out of scope here because it was cleared when retrying
  }

  rc = -5;

  if (rc == -5) {
    goto retry;
  }

  return rc;
}


int main() {
  foo(); // unsaved error
  return 0;
}
