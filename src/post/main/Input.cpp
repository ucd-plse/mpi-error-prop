#include "Input.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"

void Input::printReport(std::ostream& out, int error ATTR_UNUSED, string var, string precision ATTR_UNUSED, vector<string>& errors) {

  out << caller << ",input," << var << ',';
  
  out << '{' << errors[0];
  for(unsigned int i = 1; i < errors.size(); i++) {
    out << ' ' << errors[i];
  }
  out << '}' << endl;

  return;
}  
