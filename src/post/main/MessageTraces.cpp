#include "Message.hpp"
#include "MessageTraces.hpp"
#include <sstream>
#include <iostream>

string MessageTraces::getErrorCodeStr(bool starBefore) {
  (void) starBefore;
  return string();
}

string MessageTraces::format() {
  std::ostringstream oss;

  if (type == MessageTraces::Types::PREDICATE) {
    oss << file_name << " " << line_number << " " << target << " ";
    for(unsigned int i = 0; i < error_codes.size(); i++) {
      string code = error_codes[i];
      oss << code << " ";
    }
    oss << endl;
  }
  string ret = oss.str();
  if (!ret.empty()) {
    ret.pop_back();
  }

  return oss.str();
}

