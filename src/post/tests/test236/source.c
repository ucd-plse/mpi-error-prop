// Tests direction that is safe / error-handling for tests against a single error code
// See Github wiki for a discussion of branch pairs

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

// Pair comments are of the form (safe_branch, eh_branch)

void foo() {
	int err = bar();

	// (N,T)
	if (err == -5) {
		callA();
	}

	// (N,T)
	if (-5 == err) {
		callB();
	}

	// (N, N)
	if (err != -5) {
		callC();
	} else {
		callG();
	}

	// (N, N)
	if (-5 != err) {
		callD();
	}

	// (N, N)
	if (err == 5) {
		callE();
	}

	// (N, N)
	if (5 == err) {
		callF();
	}
}
