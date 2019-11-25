struct address_space_operations {
	int (*error_remove_page)();
};

int generic_error_remove_page() {
	return -5;
}

int generic_error_remove_page1() {
	return -6;
}

static const struct address_space_operations shmem_aops = {
	.error_remove_page = generic_error_remove_page,
};

static const struct address_space_operations shmem_aops1 = {
	.error_remove_page = generic_error_remove_page1,
};

void foo(const struct address_space_operations *aops) {
	aops->error_remove_page();
}

int main() {
	foo(&shmem_aops);
	foo(&shmem_aops1);

	return 0;
}
