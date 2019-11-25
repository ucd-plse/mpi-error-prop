

int foo() {

    int number;
    int err;

    switch(number) {
    case 0:
      err = 0; /* passing the address of arg1 */
      break;
    case 1:
      err = 1;
      break;
    default:
      err = -5;
      break;
    }
    return err;
}

int main() {
  int e = 0;

  e = foo();
  e = 0; /*overwrite*/
  return 0;
  
}
