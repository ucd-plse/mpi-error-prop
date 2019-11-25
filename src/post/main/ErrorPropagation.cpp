/*!
 * @author Cindy Rubio Gonzalez
 */

#include "ErrorPropagation.hpp"
#include "ErrorPropagationHandler.hpp"
#include "fdd.h"
#include <algorithm>
#include <sstream>


ErrorPropagation::ErrorPropagation() : BDD(bddfalse), line_number(0), is_trusted(false) {}

ErrorPropagation::ErrorPropagation( bdd b) : BDD(b), line_number(0), is_trusted(false) {}

ErrorPropagation::ErrorPropagation( bdd b, int l, string f, bool t, string fn) : BDD(b), line_number(l), file_name(f), is_trusted(t), function_name(fn) {}

ErrorPropagation::ErrorPropagation(sem_elem_t s) {
  ErrorPropagation* t = static_cast<ErrorPropagation*>(s.get_ptr());
  BDD = t->BDD;
  line_number = t->line_number;
  file_name = t->file_name;
}

ErrorPropagation::ErrorPropagation(sem_elem_t s, string f) {
  ErrorPropagation* t = static_cast<ErrorPropagation*>(s.get_ptr());
  BDD = t->BDD;
  line_number = t->line_number;
  file_name = t->file_name;
  function_name = f;
}

ErrorPropagation::~ErrorPropagation() {
}

string ErrorPropagation::getFunction() {
  return function_name;
}

sem_elem_t ErrorPropagation::one() const {
  bdd ONE = identity;
  return new ErrorPropagation(ONE);
}


sem_elem_t ErrorPropagation::zero() const {
  return new ErrorPropagation(bddfalse);
}


static bddPair *downShift;
static bddPair *upShift;

void ErrorPropagation::initialize()
{
  downShift = bdd_newpair();
  fdd_setpair(downShift, 0, 1);
  fdd_setpair(downShift, 1, 2);

  upShift = bdd_newpair();
  fdd_setpair(upShift, 2, 1);
}


sem_elem_t ErrorPropagation::extend( SemElem* se ) {

  const ErrorPropagation * const rhs = static_cast< ErrorPropagation* >(se);
  
  // 1. Shifting down bdd BDD1 dom 0 -> dom 1, dom 1 -> dom 2 
  const bdd downShifted = bdd_replace(BDD, downShift);

  // 2. Performing relational product 
  const bdd joined = bdd_relprod(downShifted, rhs->BDD, fdd_ithset(1));

  // 3. Shifting up dom 2 -> dom 1 in result
  const bdd upShifted = bdd_replace(joined, upShift);

  return new ErrorPropagation( upShifted );
}



sem_elem_t ErrorPropagation::combine( SemElem* se ) {
  ErrorPropagation* rhs = static_cast< ErrorPropagation* >(se);

  return new ErrorPropagation( bdd_or(BDD, rhs->BDD) );
}


bool ErrorPropagation::equal( SemElem* se ) const {
    ErrorPropagation* rhs = static_cast< ErrorPropagation* >(se);
	return BDD == rhs->BDD;
}


std::ostream & ErrorPropagation::print( std::ostream & o ) const {

  if (line_number != 0) {
	o << std::endl << "BDD: " << fddset << BDD << std::endl;
	o << "FILE: " << file_name << std::endl;
	o << "LINE: " << line_number << std::endl;

	if (is_trusted)
	  o << "TRUSTED." << std::endl;
	}
  return o;
}


sem_elem_t ErrorPropagation::getWeight( std::string s ) {
  if (s == "ONE")
	return one();
  else
	return zero();
}

sem_elem_t ErrorPropagation::getWeight(queue<WeightInfo>& weightInfos, int line, string file) {

  bdd weight = identity;
  bool trusted = true;
  string function;
  

  // Adding given variable relationships
  while (! weightInfos.empty()) {
	WeightInfo w = weightInfos.front();

	trusted = trusted && w.trusted;
	function = w.function;

	if ((w.weight1 == "identity") && (w.weight2 == "identity")) {
	  weightInfos.pop();
	  continue;
	}
	else if ((w.weight1 == "identityGlobals") && (w.weight2 == "identityGlobals")) {
	  weight = globalsToIdentity | constantsToIdentity | localsToUninitialized;
	}
	else if ((w.weight1 == "uninitialized") && (w.weight2 == "uninitialized")) {
	  weight = globalsToUninitialized | constantsToIdentity | localsToUninitialized;
	}

	else if ((!w.weight1.empty()) && (!w.weight2.empty())) {

	  variables->add(w.weight1);
	  variables->add(w.weight2);
	
	  int i1 = variables->getIndex(w.weight1);
	  int i2 = variables->getIndex(w.weight2);

	  int u = variables->getIndex("UNINITIALIZED");

	  weight -= (fdd_ithvar(0, i1) & fdd_ithvar(1, i1)); //removing the mapping to itself from identity bdd
	  weight -= (fdd_ithvar(0, i1) & fdd_ithvar(1, u));  //removing the mapping to uninitialize (if any)
	  weight |= (fdd_ithvar(0, i1) & fdd_ithvar(1, i2)); //adding new relation
	}
	else {
	  assert(false);
	}
	
	weightInfos.pop();
  }

  return new ErrorPropagation(weight, line, file, trusted, function);
}


MsgRef ErrorPropagation::receivesErrorMsg(int target, int source, int error, bool& slice, bool& stop) {
  MsgRef msg = Message::factory();
  bool report = line_number != 0;

  if (debugging || report) {
	const string &id_target = variables->getId(target);
	const string &id_source = variables->getId(source);

	string::size_type loc1 = id_target.find("$return", 0);
	string::size_type loc2 = id_target.find('$', 0);
	string::size_type loc3 = id_source.find("$return", 0);
	string::size_type loc4 = id_source.find('$', 0);

	if (debugging) {
      msg->debug = true;
      msg->error = variables->getId(error);
	}

	if (loc3 != string::npos) { //$return was found in source
      msg->type = Message::Types::RECEIVES_FUNCTION;
      msg->file_name = file_name;
      msg->line_number = line_number;
      msg->target = variables->getRealName(target);
      msg->function = id_source.substr(0, id_source.length() - 7);
	}
	else if (loc1 != string::npos) { //$return was found in target
      msg->file_name = file_name;
      msg->line_number = line_number;
	  if (!isError(source)) {
        msg->type = Message::Types::RECEIVES_MAYBE;
	  }
	  else {
		stop = true;
        msg->type = Message::Types::RECEIVES_IS;
        msg->error = variables->getId(error);
	  }
	}
	else if (loc2 != string::npos) { //a parameter exchange var was found
	  msg = mayHaveErrorMsg(source, error);
	}
	else if (loc4 != string::npos) { //a parameter exchange var was found in source !!!! NEW !!!!
	  msg = mayHaveErrorMsg(target, error);
	}
	else { // no exchange vars involved

	  if ((source == error) || (!isConstant(source))) {
        msg->type = Message::Types::RECEIVES_FROM;
        msg->file_name = file_name;
        msg->line_number = line_number;
        msg->target = variables->getRealName(target);
        msg->source = variables->getRealName(source);
        msg->error = variables->getId(error);
	  }
	  else {
		// recursion, LOCAL variables
		slice = false;
		msg = mayHaveErrorMsg(target, error);
	  }
	}

  }

  return msg;
}


MsgRef ErrorPropagation::mayHaveErrorMsg(int target, int error) {
  MsgRef msg = Message::factory();

  const string &starget = variables->getId(target);

  bool report = (line_number != 0);
  bool reportDebugging = debugging && (target != error);
  bool noExchange = (starget.find("$return", 0) == string::npos);

  if (reportDebugging) {
    msg->error = variables->getId(error);
    msg->debug = true;
  }

  if (reportDebugging || (report && noExchange && !isError(target))) {
    msg->type = Message::Types::MAY_HAVE;
    msg->file_name = file_name;
    msg->line_number = line_number;
    msg->target = variables->getRealName(target);
  } 
  else if (report && noExchange && isError(target)) {
	msg->type = Message::Types::RECEIVES_ARGUMENT;
	msg->file_name = file_name;
	msg->line_number = line_number;
	msg->error = variables->getRealName(error);
  }

  return msg;
}


bool ErrorPropagation::hasError(const int var, const int error) {

  /* 1. Var assigned error? */
  bdd var_with_error = bddfalse;
  //if (var != error) {
	bdd var_to_error = (fdd_ithvar(0,var) & fdd_ithvar(1,error));
	var_with_error = BDD & var_to_error;
	//}

  if (var_with_error != bddfalse) {
	return true;
  }
  return false;
}


bool ErrorPropagation::hasConstants(const int target) {

  bdd vars_assigned = fdd_ithvar(0, target) & bddtrue; //var assigned to all

  // removing OK and UNINITIALIZED
  int idxOK = variables->getIndex("OK");
  int idxNoVal = variables->getIndex("UNINITIALIZED");
  
  bdd to_remove =
	(fdd_ithvar(0, target) & fdd_ithvar(1, idxOK)) | 
	(fdd_ithvar(0, target) & fdd_ithvar(1, idxNoVal));
  
  vars_assigned = vars_assigned - to_remove;

  // removing errors
  bdd var_to_errors = bddfalse;
  vector<string>::iterator it = errorNames.begin();
  for(; it != errorNames.end(); it++) {
	int index = variables->getIndex(*it);
	var_to_errors |= (fdd_ithvar(0, target) & fdd_ithvar(1, index));
  }
  vars_assigned = vars_assigned - var_to_errors;
  vars_assigned = BDD & vars_assigned;

  if (vars_assigned == bddfalse) {
	return true;
  }
  
  return false;
}


vector<int> ErrorPropagation::getConstants(const int target) {

  vector<int> constants;  
  bdd vars_assigned = fdd_ithvar(0, target) & bddtrue; //var assigned to all

  // mapping from target to constants (assumed it only ha constants)
  vars_assigned = BDD & vars_assigned;
  
  while(vars_assigned != bddfalse) { 
	//Finding variables assigned to var
	int constant = fdd_scanvar(vars_assigned, 1);
	constants.push_back(constant);
    vars_assigned -= fdd_ithvar(0, target) & fdd_ithvar(1, constant);
  }

  return constants;
}


vector<int> ErrorPropagation::getVariables(const int target) {

  vector<int> sources;  
  bdd vars_assigned = fdd_ithvar(0, target) & bddtrue; //var assigned to all

  // removing OK and UNINITIALIZED
  int idxOK = variables->getIndex("OK");
  int idxNoVal = variables->getIndex("UNINITIALIZED");
  
  bdd to_remove =
	(fdd_ithvar(0, target) & fdd_ithvar(1, idxOK)) | 
	(fdd_ithvar(0, target) & fdd_ithvar(1, idxNoVal));
  
  vars_assigned = vars_assigned - to_remove;

  // removing errors
  bdd var_to_errors = bddfalse;
  vector<string>::iterator it = errorNames.begin();
  for(; it != errorNames.end(); it++) {
	int index = variables->getIndex(*it);
	var_to_errors |= (fdd_ithvar(0, target) & fdd_ithvar(1, index));
  }
  vars_assigned = vars_assigned - var_to_errors; 
  
  // populating vector with variables
  vars_assigned = BDD & vars_assigned;
  
  while(vars_assigned != bddfalse) { 
	//Finding variables assigned to var
	int source = fdd_scanvar(vars_assigned, 1);
	sources.push_back(source);
    vars_assigned -= fdd_ithvar(0, target) & fdd_ithvar(1, source);
  }

  return sources;
}


bool ErrorPropagation::isError(int index) {
  vector<string>::iterator it = errorNames.begin();
  for(; it != errorNames.end(); it++) {
	int errorIndex = variables->getIndex(*it);
	if (index == errorIndex) {
	  return true;
	}
  }
  return false;
}


bool ErrorPropagation::isConstant(int index) {
  vector<string>::iterator it = errorNames.begin();
  for(; it != errorNames.end(); it++) {
	int errorIndex = variables->getIndex(*it);
	if (index == errorIndex) {
	  return true;
	}
  }
  if (index == variables->getIndex("OK") || index == variables->getIndex("UNINITIALIZED")) {
	return true;
  }

  return false;
}


MsgRef ErrorPropagation::getMessage(int target, int source, int error, bool& slice, bool& stop) {
  MsgRef errorMsg = Message::factory();

  // special case for recursion
  if (!hasError(target, source)) {
	bdd targetToAll = fdd_ithvar(0, target) & bddtrue; //target mapped to everything
	bdd targetToSource = BDD & targetToAll;
	source = fdd_scanvar(targetToSource, 1);
  }

  if (target == source || ((source != error) && (isConstant(source)))) {

	// case identity
	slice = false;

	const string &idTarget = variables->getId(target);
	string::size_type loc = idTarget.find("$return", 0);
	
	if (loc != string::npos) {
	  slice = true;
	}
	errorMsg = mayHaveErrorMsg(target, error);
  }
  else if (source == error) {

	// case receiving error
	stop = true;
	slice = true;
	errorMsg = receivesErrorMsg(target, error, error, slice, stop);
  }
  else {
	// case variable

	slice = true;
	errorMsg = receivesErrorMsg(target, source, error, slice, stop);
  }

  return errorMsg;
}


/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
*/
