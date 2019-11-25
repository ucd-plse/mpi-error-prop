int main()
{
  int nonerror = -5;
  if (nonerror % 2)
    { }

  // nonerror is being used arithmetically, so quietly assume that it
  // must not have contained an error in the first place.

  return 0;
}
