
int main() {

  int error;
  int  i = 2;

  while(i > 0) {
    error = -5;
    i--;
  }

  return 0; /*error is out of scope*/
}



