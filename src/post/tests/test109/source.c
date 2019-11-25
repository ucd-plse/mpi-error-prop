/* A safe-overwrite recurring pattern in ReiserFS */

void reiserfs_warning(const char *fmt, ...) {}

int main() {
  int err, err2 = -5;
  reiserfs_warning("Warning: %s", err);
  err2 = 0; // OK to overwrite
  err = 0;  // OK to overwrite
  return 0;
}
