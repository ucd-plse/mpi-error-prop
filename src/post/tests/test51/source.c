

int main() {
  int x = 0;
  int y = -5;
  int z = 0;

  if (y == x) {
     x = -5;
  }
  else {
     x = 6;
  }
  y = 7;  // overwrite
  z = x; 
  x = 6;  // overwrite in copy mode
  y = -1;
  z = 10; // overwrite
  y = 8;  // overwrite
  return 0;
}
