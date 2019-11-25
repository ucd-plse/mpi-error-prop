#include "Assignment.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"
#include "Message.hpp"

MsgRef Assignment::getErrorMsg(int target, string msgcode, bool tentative) {
  
  string var = variables->getRealName(target);
  MsgRef msg = Message::factory();
  msg->file_name = file;
  msg->line_number = line;
  string::size_type loc = var.find("__cil", 0);
  
  string type;

  bool pointer = false;
  if (variables->isPointer(target)) {
    pointer = true;
  }

  if (msgcode == "overwrite") {
    msg->target = var;
    if (loc == string::npos) { // only report overwrites of non-temporaries
      if (tentative) {
        if (pointer)
          msg->type = Message::Types::OVERWRITE_POINTER;
        else
          msg->type = Message::Types::OVERWRITE;
      }
      else {
        if (pointer)
          msg->type = Message::Types::OVERWRITE_NT_POINTER;
        else
          msg->type = Message::Types::OVERWRITE_NT;
      }
    }

  }
  else { 
    // It is not an overwrite
    if (loc != string::npos) { //__cil was found
      // it is an unsaved error
      if (tentative) {
        msg->type = Message::Types::UNCHECK_NSAVE;
      }
      else {
        msg->type = Message::Types::UNCHECK_NT_NSAVE;
      }
    }
    else {
      msg->target = var;
      // it is an out-of-scope error
      if (tentative) {
        if (pointer)
          msg->type = Message::Types::UNCHECK_OUT_POINTER;
        else
          msg->type = Message::Types::UNCHECK_OUT;
      }
      else {
        if (pointer)
          msg->type = Message::Types::UNCHECK_NT_OUT_POINTER;
        else
          msg->type = Message::Types::UNCHECK_NT_OUT;
      }
    }
    
  }
  return msg;
  
}


void Assignment::process() {
  if (weight != NULL) {

    map<string, string>::iterator it = targets.begin();
    for(; it != targets.end(); it++) {
      
      string starget = it->first;
      int target = variables->getIndex(starget);
      int source = variables->getIndex(it->second);

      bdd targetToAllErrors = fdd_ithvar(0, target) & bddNonTentativeErrors;

      bool tentative = false;
      bdd targetToErrors = weight->BDD & targetToAllErrors;

      if (report_tentative && targetToErrors == bddfalse) { //the var does not have non-tentative errors

	// now check for tentative errors
	targetToAllErrors = fdd_ithvar(0, target) & bddTentativeErrors;
	targetToErrors = weight->BDD & targetToAllErrors;
	tentative = true; //possibly, there might not be tentative errors
      }

      if (targetToErrors != bddfalse) { //the var has at least an error, priority given to non-tentatives
	
	const int error = getError(targetToErrors);

	if (interesting(target, source, error)) {

	  string::size_type loc = starget.find("cabs2cil_", 0);
	  bool report = report_temps || (loc == string::npos);

	  if (report && line > 0) {
	    if (print != None) {	      

	      // constructing first message
	      MsgRef msg = getErrorMsg(target, msgcode, tentative);

	      if (msg->type != Message::Types::EMPTY) { // overwrites for temporaries are not reported
		
//		cout << "Error codes: ";
		vector<string> errors = getErrors(targetToErrors, target);
                msg->error = variables->getId(error);
                msg->error_codes = errors;

		// finding sample path
		Path path(msg);
		bool stop = false;
		
		FindPathVisitor visitor(target, error, error, path, stop);
		witness->accept(visitor);
		
		// printing path
		path.printReport(cout);

	      }
	    }
	  }
	}
      } // done processing a target
    } // done processing all targets

  }
}


bool Assignment::interesting(int target, int source, int error) {

  bdd targetToError = (fdd_ithvar(0,target) & (fdd_ithvar(1,error)));
  if ( (weight->BDD & targetToError) != bddfalse ) {

    bdd sourceToAllButError = (fdd_ithvar(0, source) & bddtrue) - 
      (fdd_ithvar(0, source) & (fdd_ithvar(1, error)));
    
    if ( (weight->BDD & sourceToAllButError) != bddfalse) {
      return true;
    }
    
    return false;
  }

  return false;
}
