void printk(const char [], ...);

int main() {
  int err = -5;
  printk("error!", err);
  err = 0; //overwrite should be ok
  return 0;
}

