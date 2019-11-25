int f_foo() {
	return -5;
}

int main() {
	int (*p_foo)() = f_foo;
	int err = p_foo();
	return 0;
}
