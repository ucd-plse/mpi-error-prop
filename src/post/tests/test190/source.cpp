class Source {

public:
  Source() {}

  int getError();
  void process();
  
};


int Source::getError() {
  return -5;
}

void Source::process() {
  int rv = getError();
  rv = 0;
  return;
}

int main() {

  Source *source = new Source();
  source->process();
  return 0;
}
