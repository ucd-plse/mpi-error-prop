// This test tests for correct handling of global variables in looped nested calls to global-modifying function

int a;

void bar(){
	a = 0;
}

void foo(){
	bar();
}


int main() {
  int x = 2;
  while (1){
  	if (x > 0)
  		foo();
	else 
		a = -5;
  } 
  return 0;

}
