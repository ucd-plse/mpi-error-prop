int p() {
  int d = 4;
  int e = 5;
  d = 6;
  return d;
}

int main() {
  int a;
  int b = 1;

  if (b == 3) {
    a = 2;
    p();
  }
  else
    a = 4;
  return 0;
}



