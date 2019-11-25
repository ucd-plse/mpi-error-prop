struct address_space_operations {
	int (*error_remove_page)();
};

int generic_error_remove_page() {
	return -5;
}

int generic_error_remove_page2() {
	return -6;
}

static const struct address_space_operations shmem_aops = {
	.error_remove_page = generic_error_remove_page,
};


int main() {
	int x, err;
	struct address_space_operations aops; 

	if (x) {
		aops.error_remove_page = generic_error_remove_page;
	} else {
		aops.error_remove_page = generic_error_remove_page2;
	}

	err = aops.error_remove_page();

	return 0;
}
