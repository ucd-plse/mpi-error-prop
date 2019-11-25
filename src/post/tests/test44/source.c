/* The path should not take us inside foo because err is not visible in the body of foo, it still does go inside... */

void foo() {
  int a;
  if (a > 5)
    a = 0;
  else
    a = 10;
  return;
}


int main() {

  int err = -5;
  foo();
  err = 0;

  return 0;
}

