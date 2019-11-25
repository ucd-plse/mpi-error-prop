struct address_space_operations {
	int (*error_remove_page)();
};

int generic_error_remove_page() {
	return -5;
}

int generic_error_remove_page1() {
	return -6;
}

struct address_space_operations shmem_aops = {
	.error_remove_page = generic_error_remove_page,
};

struct address_space_operations shmem_aops1 = {
	.error_remove_page = generic_error_remove_page1,
};

void foo(struct address_space_operations *aops) {
	aops->error_remove_page();
}

int main() {
	struct address_space_operations *aops;
	int x;
	if (x) {
		aops = &shmem_aops;
	} else {
		aops = &shmem_aops1;
	}
	
	foo(aops);

	return 0;
}
