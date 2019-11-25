struct s {
	int x;
	int err;
};

struct s foo(int err) {
	struct s ret;
	ret.err = err;
	return ret;
}

int main() {
	int err = foo(-5).err;
	return 0;
}
