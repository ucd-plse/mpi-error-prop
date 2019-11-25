#ifndef PREDICATE_GUARD
#define PREDICATE_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Predicate : public Values {
  
public:
  Predicate(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Predicate() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // PREDICATE_GUARD
