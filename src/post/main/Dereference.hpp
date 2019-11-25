#ifndef DEREFERENCE_GUARD
#define DEREFERENCE_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Dereference : public Values {
  
public:
  Dereference(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Dereference() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // DEREFERENCE_GUARD
