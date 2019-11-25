// Tests multiple callers for the same instance

int foo1() {
	return -5;
}

void callA();
void callB();
void callC();
void callD();

int foo2() {
	int err = foo1();
	if (err < 0) {
		callA();
		return err;
	}
	return 0;
}

void bar1() {
	int err = foo2();
	if (err < 0)  {
		callB();
	}
}

void bar2() {
	int err = foo2();
	if (!err)  {
		callC();
	}
	callD();
}
