#ifndef RULE_H
#define RULE_H

#include "VarName.hpp"
#include "Location.hpp"
#include <string>
#include <vector>

// Making these vn_t's is not a good idea unless you want
// to rewrite NameGraph.hpp. You probably don't.
struct Assignment {
  Assignment(VarName from, VarName to, bool trusted) :
    from(from), to(to), trusted(trusted) {}

  VarName from;
  VarName to;
  bool trusted;
  bool keep = true;
};

struct Rule {
  Rule() {}

  Rule(std::string from_stack) :
    from_stack(from_stack) {}

  Rule(std::string from_stack, std::string to_stack1) :
    from_stack(from_stack), to_stack1(to_stack1) {}

  Rule(std::string from_stack, std::string to_stack1, std::string to_stack2) :
    from_stack(from_stack), to_stack1(to_stack1), to_stack2(to_stack2) {}

  std::string from_stack;
  std::string to_stack1;
  std::string to_stack2;
  VarName return_value;

  Location location;
};

// Frontend rules - contain information about assignments
struct FRule : Rule {
  using Rule::Rule;

  std::string basis = "identity";
  std::string pred_op1;
  std::string pred_op2;

  void addAssignment(VarName from, VarName to, bool trusted=false);
  void addMultiAssignment(vn_t from, VarName to, bool trusted=false);

  std::string format(bool use_locations) const;
  std::vector<Assignment> assignments;
};

#endif
