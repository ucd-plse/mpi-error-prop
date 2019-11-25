/* Gotos and overwrite false positives in ext3 and ext4 */

static void *ERR_PTR(long error) {
  return (void *) error;
}

int main() {

  int rc;
  int error = -5;

 retry:
  rc = (int) ERR_PTR(error); // overwrite
  rc = -6;

  goto retry; // infinite loop, but no condition at all in purpose

  return 0;
}
