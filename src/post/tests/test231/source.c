// Mining traces
// Tests function calls inside predicate

int foo1() {
	return -5;
}

int foo2() {
	return -6;
}

void callA() {}
void callB() {}
void callC() {}

int main() {
	if (foo1() < 0) {
		callA();
	}
	if (foo2() < 0) {
		callB();
	}
	callC();
}
