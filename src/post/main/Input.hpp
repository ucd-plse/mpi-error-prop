#ifndef INPUT_GUARD
#define INPUT_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Input : public Values {
  
public:
  Input(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Input() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // INPUT_GUARD
