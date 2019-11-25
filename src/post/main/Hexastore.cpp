#include "Hexastore.hpp"
#include "FindPathVisitor.hpp"
#include "Path.hpp"

void Hexastore::process() {
  
  if (weight != NULL) {

    map<string, string>::iterator it = targets.begin();
    for(; it != targets.end(); it++) {

      // finding target and source indexes
      string starget = it->first;
      int target = variables->getIndex(starget);
      int source = variables->getIndex(it->second);

      int OK = variables->getIndex("OK");
      int uninitialized = variables->getIndex("UNINITIALIZED");

      assert(OK >= 0 && uninitialized >= 0);


      // target/source to all errors
      bdd targetToAllValues = fdd_ithvar(0, target) & bddNonTentativeErrors;
      targetToAllValues |= fdd_ithvar(0, target) & bddTentativeErrors;
      targetToAllValues |= fdd_ithvar(0, target) & fdd_ithvar(1, OK);
      targetToAllValues |= fdd_ithvar(0, target) & fdd_ithvar(1, uninitialized);

      bdd sourceToAllValues = fdd_ithvar(0, source) & bddNonTentativeErrors;
      sourceToAllValues |= fdd_ithvar(0, source) & bddTentativeErrors;
      sourceToAllValues |= fdd_ithvar(0, source) & fdd_ithvar(1, OK);
      sourceToAllValues |= fdd_ithvar(0, source) & fdd_ithvar(1, uninitialized);

      // target/source to stored errors
      bdd targetToValues = weight->BDD & targetToAllValues;
      bdd sourceToValues = weight->BDD & sourceToAllValues;
      
	    
      // producing reports
      string::size_type loc1 = starget.find("cabs2cil_", 0);
      string::size_type loc2 = starget.find("__cil_", 0);
      bool report = report_temps || (loc1 == string::npos && loc2 == string::npos);
      
      if (report && !file.empty() && line > 0) {
	
	if (targetToValues != bddfalse && sourceToValues != bddfalse) {
	  
	  //cout << "==================================================\n";

	  // printing old values for target
	  string msg = variables->getRealName(target); 
	  
	  cout << file << ',' << line << ',' << msg;
	  vector<string> errors = getErrors(targetToValues, target);

	  cout << ",{" << errors[0];

	  for(unsigned int i = 1; i < errors.size(); i++) {
	    cout << '|' << errors[i];
	  }
	  cout << '}';
	  
	  // printing new values for target (i.e., source's current values)
	  vector<string> errorsNew;
	  errorsNew = getErrors(sourceToValues, source);
	  
	  cout << ",{" << errorsNew[0];
	  for(unsigned int i = 1; i < errorsNew.size(); i++) {
	    cout << '|' << errorsNew[i];
	  }
	  cout << '}' << endl;

	  
	  
	  // Printing traces
	  /*
	  cout << "\nPaths for OLD VALUE\n";
	  for(unsigned int i = 0; i < errors.size(); i++) {
	    Path path(*this, ": variable " + variables->getRealName(target) + " contains value " + errors[i]);
	    bool stop = false;	  

	    int value = variables->getIndex(errors[i]);
	    FindPathVisitor visitor(target, value, value, path, stop);
	    witness->accept(visitor);
	    path.printReport(cout);  
	  }

	  cout << "\nPaths for NEW VALUES\n";
	  for(unsigned int i = 0; i < errorsNew.size(); i++) {

	    int value = variables->getIndex(errorsNew[i]);
	    if (source != value) { // if it is not a constant
	      Path path(*this, ": variable " + variables->getRealName(source) + " contains value " + errorsNew[i]);
	      bool stop = false;
	      
	      FindPathVisitor visitor(source, value, value, path, stop);
	      witness->accept(visitor);
	      path.printReport(cout);  
	    }
	    else {
	      cout << "Source " << variables->getRealName(source) << " is a constant\n";
	    }
	  }
	  cout << endl;
	  */

	}
      }
    } // done processing a target
  } // done processing all targets
}

