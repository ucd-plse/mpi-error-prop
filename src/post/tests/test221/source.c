typedef const struct address_space_operations aop_t;

struct address_space_operations {
	int (*error_remove_page)();
};

int generic_error_remove_page() {
	return -5;
}

static const struct address_space_operations shmem_aops = {
	.error_remove_page = generic_error_remove_page,
};


void foo(aop_t *aops) {
	int err = aops->error_remove_page();	
} 

const struct address_space_operations* get_aops() {
	return &shmem_aops;
}

int main() {
	const struct address_space_operations *aops = get_aops();
	foo(aops);
	return 0;
}
