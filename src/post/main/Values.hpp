#ifndef VALUES_GUARD
#define VALUES_GUARD 1

#include "ProgramPoint.hpp"
#include <set>

using namespace std;

class Values : public ProgramPoint {
  
public:
  Values(string f, int l, string c, string m, wali::Key k, set<string> s) :
    ProgramPoint(f, l, c, m, k), vars(s) {}

  ~Values() {}

  void process();

protected:
  set<string> vars;

private:
  bool interesting(int var, int source, int errorCode);
  virtual void printReport(std::ostream& out, int errorCode, string var, string precision, vector<string>& errors) = 0;
};


#endif // VALUES_GUARD
