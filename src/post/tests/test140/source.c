#define XFS_ERROR(e) xfs_error_trap(e)

int xfs_error_trap(int e) {

  if (!e)
    return 0;

  return e;
}


int foo() {
  int error = 5;
  
  return XFS_ERROR(5);
}

int main() {
  int err = foo();
  return 0;
}
