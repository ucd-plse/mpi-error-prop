// Test nested structures

static int ext4_symlink() {
	return -5;
}

struct inode {
	const struct inode_operations *i_op;
};

struct inode_operations {
	int (*symlink)();
};

const struct inode_operations ext4_dir_inode_operations = {
	.symlink = ext4_symlink
};

int main() {
	struct inode inode;	
	inode.i_op = &ext4_dir_inode_operations;
	int err = inode.i_op->symlink();
}
