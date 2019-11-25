// Tests errors passing through phi instructions

int foo(int lastcode, int err) {
	// This *should* generate a phi instruction
	// This test would be better if it was written directly in bitcode
	int ret = lastcode ? err : lastcode;
	return ret;
}

int main() {
	int err = foo(0, -5);
}
