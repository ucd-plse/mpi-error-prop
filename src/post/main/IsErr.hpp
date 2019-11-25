#ifndef ISERR_GUARD
#define ISERR_GUARD 1

#include "ProgramPoint.hpp"
#include <set>

using namespace std;

class IsErr : public ProgramPoint {
  
public:
  IsErr(string f, int l, string c, string m, wali::Key k, set<string> s) :
    ProgramPoint(f, l, c, m, k), vars(s) {}

  ~IsErr() {}

  void process();
  
protected:
  set<string> vars;

private:
  bool interesting(int var, int source, int errorCode);
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);
  void printWarning(std::ostream& out, string var);

};


#endif // ISERR_GUARD
