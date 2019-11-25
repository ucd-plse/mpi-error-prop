#include "ProgramPoint.hpp"
#include <cstdlib>
#include <iostream>
using namespace std;


void ProgramPoint::calculateWeight(const wali::wfa::WFA& result, const map<wali::Key, 
				   wali::wfa::ITrans*>& mapTrans) {

  map<wali::Key, wali::wfa::ITrans*>::const_iterator it = mapTrans.find(fromStkKey);

  if (it != mapTrans.end()) {
    wali::wfa::ITrans* t = it->second;

    weight_for_fromStkKey = result.getState(t->to())->weight()->extend(t->weight());
    witness = static_cast<wali::witness::Witness*>( weight_for_fromStkKey.get_ptr() );
    weight = static_cast<const ErrorPropagation*>( witness->weight().get_ptr() );
  }
}


int ProgramPoint::getError(bdd &targetToErrors) {
  int error = -1; // should not be used, as this function is called only when the target is mapped to at least one error

  if (targetToErrors != bddfalse) {
    // the variable may contain an error
    error = fdd_scanvar(targetToErrors, 1); // 2nd BDD domain
  }
  return error;
}


vector<string> ProgramPoint::getErrors(bdd targetToErrors, int const target) {

  int error;
  vector<string> errors;

  while(targetToErrors != bddfalse) {
    error = fdd_scanvar(targetToErrors, 1);
    errors.push_back(variables->getId(error));
    targetToErrors -= fdd_ithvar(0, target) & fdd_ithvar(1, error);
  }

  return errors;
}


ostream& operator << (ostream &out, const ProgramPoint &where)
{
  return out << where.file << ':' << where.line;
}
