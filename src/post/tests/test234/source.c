// Mining traces
// (Intraprocedurally) ensure that a single trace is generated 
// when a preceding condition would otherwise introduce a redundant trace

int foo() {
	int x, y;
	if (x) {}

	if (y) {
		return -5;		
	}

	return 0;
}

void bar1() {
	int err = foo();
}

void callA();

void bar() {
	if (foo() < 0) {
		callA();
	}
}
