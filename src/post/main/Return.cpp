#include "FindPathVisitor.hpp"
#include "Return.hpp"
#include "Path.hpp"


void Return::process() {

  if (weight != NULL) {

    int idxValue = variables->getIndex(value);

    if (idxValue < 0) {
      // this situation should not arise...
      cerr << "idxValue: " << idxValue << "value: " << value << endl;
      assert(false);
    }

    bdd valueToAllErrors = fdd_ithvar(0, idxValue) & bddNonTentativeErrors;
    valueToAllErrors |= fdd_ithvar(0, idxValue) & bddTentativeErrors;
    
    bdd valueToErrors = weight->BDD & valueToAllErrors;
    

    vector<string> errors;

    //if (valueToErrors != bddfalse) { // comment out if printing all traces
    while(valueToErrors != bddfalse) { // uncomment if printing all traces

      // the return value is or has an error code
      const int error = getError(valueToErrors);

      if (errors.empty()) {
	errors = getErrors(valueToErrors, idxValue);
      }
      
      MsgRef msg = Message::factory();
      msg->type = Message::Types::RETURN_FN;
      msg->file_name = file;
      msg->line_number = line;
      msg->error = variables->getId(error);
      msg->error_codes = errors;
      msg->target = caller;
      
      // creating and printing path
      if (error == idxValue) {
        Path path(msg);
        path.printReport(cout, false);
      } 
      else {

	// finding sample path
	Path path(msg);
	bool stop = false;
	FindPathVisitor visitor(idxValue, error, error, path, stop);
	witness->accept(visitor);
		
	// printing path
	path.printReport(cout);

      }

      // removing current error
      valueToErrors -= fdd_ithvar(0, idxValue) & fdd_ithvar(1, error);

    } // end while or if
  }
}
