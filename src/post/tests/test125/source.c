// Testing error-handling pattern #1

void printk(const char *fmt, ...) {}

int main( ) {
  const int *pfid;
  struct cifsFileInfo *open_file;

  int err = -5;
  int rc = -5;

  printk("Error..", err, pfid, rc, open_file);

  return 0; // nothing goes out of scope
}
