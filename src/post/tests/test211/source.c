int foo() {
	return -5;
}

int main() {
	int *x;
	*x = foo();
}
