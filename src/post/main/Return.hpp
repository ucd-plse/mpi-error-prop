#ifndef RETURN_GUARD
#define RETURN_GUARD 1

#include "ProgramPoint.hpp"

class Return : public ProgramPoint {
  
public:
  Return(string f, int l, string c, string m, wali::Key k, string v) :
    ProgramPoint(f, l, c, m, k), value(v) {}

  ~Return() {}

  void process();

protected:
  string value;

private:
  bool interesting(int var, int source, int errorCode);

};


#endif // RETURN_GUARD
