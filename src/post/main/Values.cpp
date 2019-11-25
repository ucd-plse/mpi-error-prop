#include "Values.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"

void Values::process() {

  if (weight != NULL) {

    set<string>::iterator it;
    for(it = vars.begin(); it != vars.end(); it++) {
      string variable = *it;

      int idxVariable = variables->getIndex(variable);
      
      if (idxVariable < 0) {
	// this situation should not arise...
	cerr << "idxVariable: " << idxVariable << "value: " << variable << endl;
	assert(false);
      }
      
      bdd variableToAllErrors = fdd_ithvar(0, idxVariable) & bddNonTentativeErrors;
      variableToAllErrors |= fdd_ithvar(0, idxVariable) & bddTentativeErrors;
      
      bdd variableToErrors = weight->BDD & variableToAllErrors;
    
      
      int indexOK = variables->getIndex("OK");
      bdd bddOK = fdd_ithvar(1, indexOK);
      bdd varToOK = fdd_ithvar(0, idxVariable) & bddOK;
      string precision = "must";
      
      if ((weight->BDD & varToOK) != bddfalse) {
	precision = "may";
      }
      
      if (variableToErrors != bddfalse) {
	const int error = getError(variableToErrors);
	vector<string> errors = getErrors(variableToErrors, idxVariable);
	printReport(cout, error, variable, precision, errors);
      }

    } // end for
  } // end if

  return;
}
