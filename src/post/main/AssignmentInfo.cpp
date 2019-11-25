#include "AssignmentInfo.hpp"


void AssignmentInfo::process() {
  
  if (weight != NULL) {

    map<string, string>::iterator it = targets.begin();
    for(; it != targets.end(); it++) {

      // finding target and source indexes
      string starget = it->first;
      int target = variables->getIndex(starget);
      int source = variables->getIndex(it->second);

      // target/source to all errors
      bdd targetToAllErrors = fdd_ithvar(0, target) & bddNonTentativeErrors;
      targetToAllErrors |= fdd_ithvar(0, target) & bddTentativeErrors;

      bdd sourceToAllErrors = fdd_ithvar(0, source) & bddNonTentativeErrors;
      sourceToAllErrors |= fdd_ithvar(0, source) & bddTentativeErrors;


      // target/source to stored errors
      bdd targetToErrors = weight->BDD & targetToAllErrors;
      bdd sourceToErrors = weight->BDD & sourceToAllErrors;


      // producing reports
      string::size_type loc1 = starget.find("cabs2cil_", 0);
      string::size_type loc2 = starget.find("__cil_", 0);
      bool report = report_temps || (loc1 == string::npos && loc2 == string::npos);
      
      if (report) {
	
	if (targetToErrors != bddfalse && sourceToErrors != bddfalse) {

	  if (csv) {
	    vector<string> target_errors = getErrors(targetToErrors, target);
	    vector<string> source_errors = getErrors(sourceToErrors, source);

	    for(unsigned int i = 0; i < target_errors.size(); i++) {
	      // print target
	      cout << target_errors[i];

	      // iterate through sources
	      for(unsigned int j = 0; j < source_errors.size(); j++) {
		cout << ',' << source_errors[j];
	      }
	      cout << endl;
	    }
	  }
	  else {
	    // default format

	    // printing target data
	    string msg = "{target:" + variables->getRealName(target) + ':';	      
	    
	    cout << *this << msg;
	    vector<string> errors = getErrors(targetToErrors, target);
	    
	    cout << errors[0];
	    for(unsigned int i = 1; i < errors.size(); i++) {
	      cout << ',' << errors[i];
	    }
	    cout << '}' << endl;	      
	    
	    
	    // printing source data
	    msg = "{source:" + variables->getRealName(source) + ':';	      
	    
	    cout << *this << msg;
	    errors.clear();
	    errors = getErrors(sourceToErrors, source);
	    
	    cout << errors[0];
	    for(unsigned int i = 1; i < errors.size(); i++) {
	      cout << ',' << errors[i];
	    }
	    cout << '}' << endl;
	  }
	}

      } // done processing a target
    } // done processing all targets

  }
}

