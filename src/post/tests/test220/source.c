struct address_space_operations {
	int (*error_remove_page)();
};

int generic_error_remove_page() {
	return -5;
}

static const struct address_space_operations shmem_aops = {
	.error_remove_page = generic_error_remove_page,
};


int main() {
	int err = shmem_aops.error_remove_page();

	return 0;
}
