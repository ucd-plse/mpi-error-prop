int main()
{
  int error = -5;
  int nonerror = -5;

  int sum = nonerror + !error;

  // nonerror is being used arithmetically, so quietly assume that it
  // must not have contained an error in the first place.

  // error is being used logically, not arithmetically, so it may
  // contain an error.  This error is not checked before the function
  // exits.

  // sum is the result of a calculation, so its not an error either

  return 0;
}
