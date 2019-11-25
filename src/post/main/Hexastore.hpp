#ifndef HEXASTORE_GUARD
#define HEXASTORE_GUARD 1

#include "ProgramPoint.hpp"

class Hexastore : public ProgramPoint {
  
public:
  Hexastore(string f, int l, string c, string m, wali::Key k, map<string, string> t) :
    ProgramPoint(f, l, c, m, k), targets(t) {}

  ~Hexastore() {}

  void process();

protected:
  map<string, string> targets;

};


#endif // HEXASTORE_GUARD
