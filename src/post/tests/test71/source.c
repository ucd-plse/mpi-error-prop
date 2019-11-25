int main()
{
  int error = -5;
  if (error < 0)
    { }

  // error is not being used arithmetically, so it may have an
  // unchecked error.

  return 0;
}
