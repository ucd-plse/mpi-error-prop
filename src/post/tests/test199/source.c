// This test tests for correct handling of local variables in  recursive calls
int foo(){
  int ret = 0;
  int bar = 0;
  ret = -5;
  if (bar == 0) { 
  	ret = foo();
  }
  ret = -5;
  return bar; 
}

int main() {
  int x = -5;
  x = 6;
  int y = foo();
  return 0;
}
