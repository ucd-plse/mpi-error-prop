int nextId() {
  static int id;
  return ++id;
}

int getError() {
  return -5;
}

int main() {
  int status, result = 0;

  if (nextId())
    status = getError();

  result = status;

  if (nextId())
    result = -6;

  return result;
}
