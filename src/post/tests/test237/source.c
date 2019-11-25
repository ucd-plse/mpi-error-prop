// Tests that traces are generated for predicates that test a single error code

int bar() {
	return -5;
}

void callA();
void callB();
void callC();
void callD();
void callE();
void callF();
void callG();

void foo() {
	int err = bar();

	if (err == -5) {
		// Should show up
		callA();
	}

	if (-5 == err) {
		// Should show up
		callB();
	}

	// (N, N)
	if (err != -5) {
		// Should not show up
		callC();
	} else {
		// Should not show up
		callG();
	}

	// (N, N)
	if (-5 != err) {
		// Should not show up
		callD();
	}

	// (N, N)
	if (err == 5) {
		// Should not show up
		callE();
	}

	// (N, N)
	if (5 == err) {
		// Should not show up
		callF();
	}
}
