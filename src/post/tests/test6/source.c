#include <stdio.h>

int main() {
  int x = -5;
  int y = -5;

  x = -67737868; //x is checked
  y = x;  //y was not checked
  x = 7;  //OK

  return 0;
}
