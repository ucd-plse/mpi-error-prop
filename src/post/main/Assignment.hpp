#ifndef ASSIGNMENT_GUARD
#define ASSIGNMENT_GUARD 1

#include "ProgramPoint.hpp"

class Assignment : public ProgramPoint {
  
public:
  Assignment(string f, int l, string c, string m, wali::Key k, map<string, string> t) :
    ProgramPoint(f, l, c, m, k), targets(t) {}

  ~Assignment() {}

  void process();

protected:
  map<string, string> targets;


private:
  MsgRef getErrorMsg(int var, string msgcode, bool tentative);
  bool interesting(int var, int source, int errorCode);

};


#endif // ASSIGNMENT_GUARD
