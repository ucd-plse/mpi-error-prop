/* First attemp to simulate the artificial main but it was not quite the same */

#include <stdio.h>


int foo(int* err, int number) {
  *err = -5;

  if (number > 0) 
    printf("blabla...\n");
  else
    *err = 0; /* overwrites error */

  return *err;
}

void foo1() {
  int err = -5;
  err = 0;  /* overwrites error */
  return;
}


int main() {
  while(1) {
    int number;
    int arg1;
    int arg2;

    switch(number) {
    case 0:
      foo(&arg1, arg2); /* passing the address of arg1 */
      break;
    case 1:
      foo1();
      break;
      default:
      // no calls
      break;
    }
  }

  return 0;
}
