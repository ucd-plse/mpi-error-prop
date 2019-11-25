int user_path_at() {
  return -5;
}

int inode_permission() {
  return -5;
}

void path_put();

int chdir() {
  int error;
  int x;

retry:
  error = user_path_at();
  if (error)
    goto out;

  error = inode_permission();
  if (error)
    goto dput_and_out;

dput_and_out:
  path_put();
  if (x)
    goto retry;

out:
  return error;
}
