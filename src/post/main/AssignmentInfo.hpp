#ifndef ASSIGNMENTINFO_GUARD
#define ASSIGNMENTINFO_GUARD 1

#include "ProgramPoint.hpp"

class AssignmentInfo : public ProgramPoint {
  
public:
  AssignmentInfo(string f, int l, string c, string m, wali::Key k, map<string, string> t) :
    ProgramPoint(f, l, c, m, k), targets(t) {}

  ~AssignmentInfo() {}

  void process();

protected:
  map<string, string> targets;

};


#endif // ASSIGNMENTINFO_GUARD
