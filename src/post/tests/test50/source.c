/* The variable containing the error code appears as part 
of an if conditional and is therefore considered checked */

int main() {
  int err = -5;

  if (err == -5) {
  }

  err = 0; /*Overwriting error*/
  
  return 0;
}



