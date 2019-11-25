#ifndef DATAFLOWWALI_HPP
#define DATAFLOWWALI_HPP

#include "Dataflow.hpp"
#include "VarName.hpp"
#include "wali/SemElem.hpp"
#include <map>

using wali::SemElem;
using wali::sem_elem_t;

class DataflowWali : public Dataflow {
public:
  DataflowWali(bool only_preds=false) : only_preds(only_preds) {}
  virtual DataflowResult solve(std::vector<FRule> rules);

private:
  bool only_preds;
};

class PossibleErrorCodes : public wali::SemElem {  

public:
  PossibleErrorCodes() : one_zero(true) {}

  PossibleErrorCodes(FRule rule);

  sem_elem_t one() const;
  sem_elem_t zero() const;
  
  sem_elem_t extend(SemElem* rhs);
  sem_elem_t combine(SemElem* rhs);
  bool equal(SemElem *rhs) const;
  
  std::ostream& print(std::ostream &o) const;

  var_map get_codes() const { return codes;}

  static unsigned long count;
  static unsigned long create_count;
  static wali::sem_elem_t ONE;


private:
  var_map codes;
  // ONE if true, ZERO if false
  bool one_zero;
};

#endif
