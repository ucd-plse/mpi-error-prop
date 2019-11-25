class Source {

public:
  Source() {}

  int getError();
  void process();

protected:
  void doNothing1(int x);

private:
  void doNothing2();
  
};


int Source::getError() {
  return -5;
}

void Source::process() {
  int rv = getError();
  rv = 0;
  return;
}

void Source::doNothing1(int x) {
  return;
}

void Source::doNothing2() {
  return;
}





