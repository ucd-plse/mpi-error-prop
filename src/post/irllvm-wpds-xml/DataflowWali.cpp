#include "DataflowWali.hpp"
#include "Location.hpp"
#include "wali/SemElem.hpp"
#include "wali/wpds/WPDS.hpp"
#include "wali/wpds/fwpds/FWPDS.hpp"
#include "wali/wfa/WFA.hpp"

using namespace std;
using wali::SemElem;
using wali::sem_elem_t;
using wali::wfa::WFA;

unsigned long PossibleErrorCodes::count = 0;
unsigned long PossibleErrorCodes::create_count = 0;
wali::sem_elem_t ONE(new PossibleErrorCodes());

DataflowResult DataflowWali::solve(vector<FRule> rules) {
  // This treats each error code instance separately
  // We might need to change that if this is ever used for something other than traces
  // Also, it uses the same name mangling hack that the RulesPrinter does
  // We could use an actual (Name, Loc) struct here, but... another day.

  DataflowResult res;

  wali::wpds::fwpds::FWPDS wpds;

  wali::Key q = wali::getKey("q");
  wali::Key accepting_state = wali::getKey("accepting_state");
  wali::Key main = wali::getKey("main.0");

  wali::sem_elem_t ONE(new PossibleErrorCodes());
  
  map<string, wali::Key> stack_keys;
  map<string, FRule> stack_rules;

  cerr << "There are " << rules.size() << " rules to process." << endl;
  for (FRule r : rules) {
    PossibleErrorCodes *weight = new PossibleErrorCodes(r);    

    wali::Key from_key = wali::getKey(r.from_stack);
    stack_keys[r.from_stack] = from_key; 
    stack_rules[r.from_stack] = r;

    if (r.to_stack1.empty()) {
      // Pop rule
      wpds.add_rule(q, from_key, q, weight);
    }
    else {
      wali::Key to1_key  = wali::getKey(r.to_stack1);
      stack_keys[r.to_stack1] = to1_key;
      stack_rules[r.to_stack1] = r;
      if (r.to_stack2.empty()) {
	wpds.add_rule(q, from_key, q, to1_key, weight);
      } else {
	wali::Key to2_key = wali::getKey(r.to_stack2);
	stack_keys[r.to_stack2] = to2_key;
	stack_rules[r.to_stack2] = r;
	wpds.add_rule(q, from_key, q, to1_key, to2_key, weight);
      }
    }
  }

  WFA query;
  query.addTrans(q, main, accepting_state, ONE);
  query.addFinalState(accepting_state);
  query.setInitialState(q);

  WFA answer;
  wpds.poststar(query, answer);

  cerr << "Inspecting program points...\n";

  for (map<string, wali::Key>::const_iterator ki = stack_keys.begin(), ke = stack_keys.end(); ki != ke; ++ki) {
    wali::wfa::Trans goal;

    FRule r = stack_rules[ki->first];
    if (!only_preds || !r.pred_op1.empty() || !r.pred_op2.empty()) {
      cerr << ki->first << " is interesting\n";
      answer.find(q, ki->second, accepting_state, goal);
      SemElem *se = &*goal.weight();
      if (se) {
	PossibleErrorCodes *weight = static_cast<PossibleErrorCodes*>(se);      
	res.addVarMap(ki->first, stack_rules[ki->first], weight->get_codes());
      }
    }
  }

  return res;
}

PossibleErrorCodes::PossibleErrorCodes(FRule rule) : one_zero(true) {  
  for (const Assignment &a : rule.assignments) {
    VarName from = a.from;
    string from_name = from.name();
    
    if (from_name.find("TENTATIVE_") != string::npos) {
      from_name += "!" + rule.location.file + ":" + std::to_string(rule.location.line);
    }
    from.setName(from_name);

    set<VarName> from_set = { from };
    codes[a.to] = from_set;
  }
}

sem_elem_t PossibleErrorCodes::one() const {
  return new PossibleErrorCodes();
}

sem_elem_t PossibleErrorCodes::zero() const {
  PossibleErrorCodes *ret = new PossibleErrorCodes();
  ret->one_zero = false;
  return ret;
}

sem_elem_t PossibleErrorCodes::extend(SemElem *se) {
  PossibleErrorCodes *rhs = static_cast<PossibleErrorCodes*>(se);

  if (!rhs->one_zero) {
    return sem_elem_t(rhs);
  }

  for (var_map::const_iterator rhs_it = rhs->codes.begin(), rhs_e = rhs->codes.end(); rhs_it != rhs_e; ++rhs_it) {
    // The variable potentially holding an error code in the RHS
    VarName var = rhs_it->first;
    std::set<VarName> rhs_values = rhs_it->second;

    codes[var] = std::set<VarName>();
    std::set<VarName>::const_iterator val_it = rhs_values.begin(), val_e = rhs_values.end();
    for (; val_it != val_e; ++val_it) {
      VarName value = *val_it;
      if (codes.find(value) != codes.end()) {
	// Should this be overwrite?
	codes[var].insert(codes[value].begin(), codes[value].end());
      } else {
	codes[var].insert(rhs_values.begin(), rhs_values.end());
      }
    }
  }

  return wali::sem_elem_t(this);
}

sem_elem_t PossibleErrorCodes::combine(SemElem *se) {
  PossibleErrorCodes *rhs = static_cast<PossibleErrorCodes*>(se);

  for (var_map::const_iterator rhs_it = rhs->codes.begin(), rhs_e = rhs->codes.end(); rhs_it != rhs_e; ++rhs_it) {
    codes[rhs_it->first].insert(rhs_it->second.begin(), rhs_it->second.end());
  }

  return wali::sem_elem_t(this);
}

bool PossibleErrorCodes::equal(SemElem *se) const {
  PossibleErrorCodes *rhs = static_cast<PossibleErrorCodes*>(se);

  if (one_zero != rhs->one_zero) {	   
    return false;
  }

  return codes == rhs->codes;
}

std::ostream& PossibleErrorCodes::print(std::ostream &o) const {
  if (! one_zero) {
    o << "ZERO\n";
    return o;   
  }

  if (one_zero) {
    o << "CODES = ";
    for (var_map::const_iterator it = codes.begin(), it_end = codes.end(); it != it_end; ++it) {
      o << it->first.name() << ": ";
      const std::set<VarName> &errors = it->second;
      for (std::set<VarName>::const_iterator ec_it = errors.begin(), ec_it_end = errors.end(); 
	   ec_it != ec_it_end; 
	   ++ec_it) {
	o << ec_it->name() << " ";
      }
    }
    o << endl;
    return o;
  }

  return o;
}
