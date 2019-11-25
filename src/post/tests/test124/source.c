// Testing error-handling pattern #3

struct cifsFileInfo {};

void cERROR(int button, const char *fmt) {}

int main( ) {
  const int *pfid;
  struct cifsFileInfo *open_file;

  int err = -5;
  int rc = -5;

  err = 0; // overwrite

  if (open_file) {

  } else if (pfid == 0) {
    if (rc != 0) {
      cERROR(1, ("Error ... "));
      return 0; // rc is NOT out of scope
    }
  }

  return 0; // rc is out of scope
}
