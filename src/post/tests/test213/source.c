int foo() {
	int x;
	return x ? -5 : 0;
}

int main() {
	int err = foo();
	return 0;	
}
