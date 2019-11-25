// Mining traces
// No tests against error instance - should report nothing

int foo1() {
	return -5;
}

int foo2() {
	return foo1();
}

int bar() {
	int x;
		
	if (x) {
		int err = foo1();
	} else {
		int err = foo2();
	}
	int err = foo1();

	return 0;
}
