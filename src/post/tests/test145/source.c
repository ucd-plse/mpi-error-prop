
// Original test: 108

static void *ERR_PTR(long error) {
  return (void *) error;
}


int main() {

  long* acl; /*the type does not really matter*/
  int retval = -5;

  if (retval < 0) {
    acl = ERR_PTR(retval);
  }

  return 0; // copy: retval and acl is out of scope, tranfer: acl out of scope
}
