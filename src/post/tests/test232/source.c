// Mining traces
// Tests variable that may hold two different values
// Tests additional branching on the else branch

int foo1() {
	return -5;
}

int foo2() {
	return -6;
}

int prop() {
	return foo1();
}

int main() {
	int x, err;
	if (x) {
		err = foo1();	
	} else {
		err = foo2();
	}

	if (x) {
	} else {
		if (err) {
			foo1();
		}
	}

	return 0;
}
