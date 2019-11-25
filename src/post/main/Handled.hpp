#ifndef HANDLED_GUARD
#define HANDLED_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Handled : public Values {
  
public:
  Handled(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Handled() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // HANDLED_GUARD
