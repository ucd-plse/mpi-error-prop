#ifndef OUTPUT_GUARD
#define OUTPUT_GUARD 1

#include "Values.hpp"
#include <set>

using namespace std;

class Output : public Values {
  
public:
  Output(string f, int l, string c, string m, wali::Key k, set<string> s) :
    Values(f, l, c, m, k, s) {}

  ~Output() {}

private:
  void printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors);

};


#endif // OUTPUT_GUARD
