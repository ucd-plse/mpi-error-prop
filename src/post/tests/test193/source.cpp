struct Base
{
  virtual void slot0() const;
  virtual int getError() const;
};

struct Derived : public Base
{
  int getError() const;
};


void Base::slot0() const
{
}


int Base::getError() const
{
  return 0;
}


int Derived::getError() const
{
  return -5;
}


volatile int unknown;


int main()
{
  Base * const base = unknown ? new Base() : new Derived();
  base->getError();
  return 0;
}
