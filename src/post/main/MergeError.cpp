/*!
 * @author Cindy Rubio Gonzalez
 */

#include "MergeError.hpp"
#include "ErrorPropagation.hpp"
#include "ErrorPropagationHandler.hpp"
#include "fdd.h"
#include "wali/ref_ptr.hpp"
#include <vector>

using namespace std;


MergeError::MergeError(std::string f)
  : function(f)
{
}

MergeError::MergeError(std::string f, sem_elem_t w)
  : function(f), weight(w)
{
}

// change this!
merge_fn_t MergeError::getMergeFn( std::string f) {
  return new MergeError(f);
}


SemElem* MergeError::apply_f(SemElem* w1, SemElem* w2) {

  if (weight == NULL) {
	assert(0);
  }

  ErrorPropagation* ew11 = static_cast<ErrorPropagation*>(w1);
  ErrorPropagation* ew2 = static_cast<ErrorPropagation*>(w2);

  // weight before call + call itself
  ErrorPropagation ew1(ew11->extend(weight.get_ptr()));

  ErrorPropagation ew2_new( bdd_and(globalsToUniverse, ew2->BDD) ); //before calling the other constructor
  ew2_new.BDD |= localsToIdentity;
  ew2_new.BDD |= constantsToIdentity;

  // transforming errors from tentative to non-tentative
  if (!hexastore) {
  
	int index = variables->getIndex(function + "$return");
	if (index != -1) { // void functions do not have return exchange vars
	  bdd bddVarToTentative = fdd_ithvar(0, index) & bddTentativeErrors;
	  
	  if ((ew2_new.BDD & bddVarToTentative) != bddfalse) {
		
		for(int unsigned i = 0; i < tentativeErrorNames.size(); i++) {
		  
		  int indexTentative = variables->getIndex(tentativeErrorNames[i]);
		  bdd varToTentative = fdd_ithvar(0, index) & fdd_ithvar(1, indexTentative);
		  
		  if ((ew2_new.BDD & varToTentative) != bddfalse) {
			ew2_new.BDD -= varToTentative;
		  int indexReal = mapTentativeToReal[indexTentative];
		  bdd varToReal = fdd_ithvar(0, index) & fdd_ithvar(1, indexReal);
		  ew2_new.BDD |= varToReal;
		  }
		}
	  }
	}

  } // !hexastore

  ErrorPropagation* result = new ErrorPropagation(ew1.extend(&ew2_new), function);
  //std::cout << "result: "; result->print(std::cout);

  return result; 
  
  
  //Default implementation where extend is performed
  /*
  sem_elem_t r = w1->extend(w2);
  return new ErrorPropagation(r);
  */
}


sem_elem_t MergeError::apply_f(sem_elem_t w1, sem_elem_t w2)
{
  return apply_f(w1.get_ptr(), w2.get_ptr());
}


/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
*/
