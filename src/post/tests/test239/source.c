int foo() {
	return -5;
}

struct S {
	int x;
	int y;
};

int main() {
	struct S st;
	
	int err1 = foo();
	int err2 = foo();

	if (err1) goto fail1;

	st.x = 10;
	st.y = 10;

	if (err2) goto fail2;

fail2:
	st.x = 0;

fail1:
	return 0;	
	
}
