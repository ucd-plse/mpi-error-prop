#ifndef OPERAND_GUARD
#define OPERAND_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Operand : public Values {
  
public:
  Operand(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Operand() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // OPERAND_GUARD
