//
// Testing overwrite location after infinite loop

int a;

int foo(){
	a = -2;
	int c = 0;
	while (1){
		c = 5;
	}
	return 0;
}


int main() {
  int c = foo();
  a = 0;
  
  return 0;

}
