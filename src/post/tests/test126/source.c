// Testing error-handling pattern #2

void ext3_warning() {}

int main() {
  int error = -5;
  ext3_warning();
  error = 0; // no overwrite
  return 0;
}
