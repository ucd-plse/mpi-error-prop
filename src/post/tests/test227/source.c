int foo() {
	return -5;
}

void bar() {}

int main() {
	int err = foo();

	if (err < 0) {
		bar();
	}
	
	return 0;
}
