/* Dataflow interface
 *  
 * The Rules class calls the solve() method
 * Any alternate dataflow approach should implement solve 
 */

#ifndef DATAFLOW_HPP
#define DATAFLOW_HPP

#include <map>
#include <string>
#include <set>
#include "Rule.hpp"
#include "VarName.hpp"

typedef std::map<VarName, std::set<VarName>> var_map;
typedef std::map<std::string, var_map> dflow_stack_map;
typedef std::pair<dflow_stack_map::const_iterator, dflow_stack_map::const_iterator> dflow_stack_range;

class DataflowResult {
public:
  var_map getVars(std::string stack) const { 
    if (stack2vars.find(stack) != stack2vars.end()) {
      return stack2vars.at(stack); 
    } else {
      return std::map<VarName, std::set<VarName>>();
    }
  }
  FRule getRule(std::string stack) const { return stack2rule.at(stack); }

  void addVarMap(std::string stack, FRule rule, var_map map) {
    stack2vars[stack] = map;
    stack2rule[stack] = rule;
  }

  // (begin, end) range iterator
  dflow_stack_range getStackRange() const {
    return make_pair(stack2vars.begin(), stack2vars.end());
  }
  

private:
  std::map<std::string, var_map> stack2vars;
  std::map<std::string, FRule> stack2rule;
};

class Dataflow {

  // Takes the vector of rules and returns vector of (stack location, set of (variable name, set of values)) pairs
  virtual DataflowResult solve(std::vector<FRule> rules) = 0;
};


#endif
