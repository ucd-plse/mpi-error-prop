int foo() {
	int x;
	if (x > 0) {
		return -5;
	}
	return 0;
}

void callA();

int main() {
	int err = foo();

	if (err < 0) {
		callA();
	}
	return 0;
}
