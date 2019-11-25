#include "IsErr.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"
#include <sstream>


void IsErr::process() {

  if (weight != NULL) {

    set<string>::iterator it;
    for(it = vars.begin(); it != vars.end(); it++) {
      string variable = *it;

      int idxVariable = variables->getIndex(variable);
      
      if (idxVariable < 0) {
	// the variable does not have a mapping, thus no error codes
	printWarning(cout, variable);
      }
      else {
      
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
	else {
	  // the variable has a mapping, but does not contain any error codes
	  printWarning(cout, variable);
	}
      }
    } // end for
  } // end if

  return;
}


void IsErr::printReport(std::ostream& out, int error, string var, string precision, vector<string>& errors) {
  
  MsgRef msg = Message::factory();
  msg->type = Message::Types::ISERR;
  msg->file_name = file;
  msg->line_number = line;
  msg->target = var;
  msg->precision = precision;
  msg->error = variables->getId(error);
  msg->error_codes = errors;
  
  // finding sample path
  Path path(msg);
  bool stop = false;
  int idxVariable = variables->getIndex(var);
  FindPathVisitor visitor(idxVariable, error, error, path, stop);
  witness->accept(visitor);
  
  // printing path
  path.printReport(out);
  
  return;
}


void IsErr::printWarning(std::ostream& out, string var) {

  MsgRef msg = Message::factory();
  msg->type = Message::Types::ISERRWARN;
  msg->file_name = file;
  msg->line_number = line;
  msg->target = var;

  Path path(msg);
  path.printReport(out, false);

  return;
}
