int main()
{
  int error = -5;
  int nonerror = -5;

  error = nonerror + 2;

  // nonerror is being used arithmetically, so quietly assume that it
  // must not have contained an error in the first place.

  // error is being overwritten with the result of an arithmetic
  // calculation, which is assumed to be a non-error.  Therefore, this
  // assignment might overwrite an error with a non-error.

  return 0;
}
