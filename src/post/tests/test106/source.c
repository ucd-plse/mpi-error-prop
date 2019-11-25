void *ERR_PTR(int);

int main() {

  long* acl; /*the type does not really matter*/
  int retval = -5;

  if (retval < 0) {
    acl = ERR_PTR(retval); // retval is cleared, acl is not tracked down
  }

  return 0;
}
