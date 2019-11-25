/*
 * Recurring safe-overwrite pattern in ReiserFS
 */

void printk(const char [], ...);

int main() {
  int retval = -5;
  int err = -6;

  if (retval) {
    if (err) {
      retval = err; /*retval is overwritten*/
    }
  }

  printk("Error...", retval);
  printk("Error...", err);

  return 0;
}
