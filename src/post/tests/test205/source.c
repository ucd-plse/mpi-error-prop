// Testing "old" value before call into infinite loop

int a;

int bar(){
	a = -2;
	int c = 0;
	while (1){
		c = 5;
	}
	return 0;
}

int foo(){
	a = -3;
	bar();
	return 0;
}



int main() {
  int c = 0;
  if (c > 3)
  	foo();
  a = 0;
  
  return 0;

}
