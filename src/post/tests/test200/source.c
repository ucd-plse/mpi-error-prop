
#define NS_FAILED(_nsresult)    _nsresult
#define NS_SUCCEEDED(_nsresult) !_nsresult

//#define NS_FAILED(_nsresult)    (NS_UNLIKELY((_nsresult) & 0x80000000))
//#define NS_SUCCEEDED(_nsresult) (NS_LIKELY(!((_nsresult) & 0x80000000)))


int foo() {
  int x = -5;
  int y = -6;

  if (NS_FAILED(x)) {
    return x; // wrong "x" out-of-scope report because of temporary variable (TODO: fix in AssignmentsReturnBlock)
  }

  if (NS_SUCCEEDED(y)) {
    y = 0; // this should NOT be an overwrite!
  }

  y = 0; // this is an overwrite!

  x = 0; // this should NOT be an overwrite! No, we still report it.

  return 0;
}


int main() {
  foo(); // unsaved error
  return 0;
}
