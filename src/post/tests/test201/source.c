// This test tests for correct handling of local variables in  recursive calls

int x;

void bar() {
	int a = -5;
	a = 0;
}

void foo(){
	int b = -2;
	bar();
	b = 0;
}

int main() {
  x = -5;
  foo();
  foo();
  x = 0;
  return 0;

}
