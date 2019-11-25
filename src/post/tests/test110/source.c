void ext3_warning(const char *fmt, ...) {}

void ext3_error(const char *fmt, ...) {}

void ea_bdebug(const char *fmt, ...) {}

void dummy(const char *fmt, ...) {}


int main() {
  int err0;
  int err1 = -5, err2 = -5;
  ext3_warning("warning: %s", err1, err0);
  err2 = 0; // OK to overwrite
  err1 = 0; // OK to overwrite

  int err3 = -5, err4 = -5;
  ext3_error("error: %s", err3);
  err4 = 0; // OK to overwrite
  err3 = 0; // OK to overwrite

  int err5 = -5, err6 = -5;
  ea_bdebug("error: %s", err5);
  err6 = 0; // OK to overwrite
  err5 = 0; // OK to overwrite

  int err7 = -5, err8 = -5;
  dummy("dummy: %s", err7);
  err8 = 0; // Overwrite!
  err7 = 0; // Overwrite!
  
  return 0;
}
