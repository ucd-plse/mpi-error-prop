/*!
 * @author Nick Kidd/ modified by Cindy
 */

#include "ErrorPropagationHandler.hpp"
#include "FindPathVisitor.hpp"
#include "wali/witness/Witness.hpp"
#include "wali/witness/WitnessExtend.hpp"
#include "wali/witness/WitnessCombine.hpp"
#include "wali/witness/WitnessRule.hpp"
#include "wali/witness/WitnessTrans.hpp"
#include "wali/witness/WitnessMerge.hpp"
#include "Path.hpp"
#include "Variables.hpp"
#include "Message.hpp"


typedef wali::witness::witness_t witness_t;
typedef std::list< wali::witness::witness_t > witness_ls_t;


FindPathVisitor::FindPathVisitor(int v, int s, int e, Path& p, bool& f) :
  target(v),source(s),error(e),path(p),stop(f) {}

FindPathVisitor::~FindPathVisitor() {}

/*!
 * @return true to continue visiting children, false to stop
 */
bool FindPathVisitor::visit( wali::witness::Witness * w ATTR_UNUSED) {
  return true;
}


/*!
 * @return true to continue visiting children, false to stop.
 */
bool FindPathVisitor::visitExtend( wali::witness::WitnessExtend * w ) {

  witness_t r = w->right();
  witness_t l = w->left();
  
  ErrorPropagation *wr = witnessToWeight(r.get_ptr());
  ErrorPropagation *wl = witnessToWeight(l.get_ptr());

  int idxU = variables->getIndex("UNINITIALIZED");
  
  if( w->hasLeft() && w->hasRight() ) {


	if (source == idxU) {

	  r->accept(*this);
	  l->accept(*this);
	}

	else if (wr->hasError(target, source)) {

	  // mapped to source
	  
	  if (! path.isComplete()) {
		r->accept(*this);
		
		if (!path.isComplete()) {
		  if (wl->hasError(target, source)) {
			l->accept(*this);
		  }
		  else {
            FindPathVisitor lpath(target, target, error, path, stop);
            l->accept(lpath);
		  }
		}
	  }
	}
	else {
	  
	  // not mapped to source
	  
	  vector<int> newSources = wr->getVariables(target);
	  vector<int>::iterator it = newSources.begin();
	  
	  for(; it != newSources.end(); it++) {

		int var = *it;

		if (wl->hasError(var, source)) {

		  if (! path.isComplete()) {
			FindPathVisitor rpath(target, var, error, path, stop);
			r->accept(rpath);

			if (!path.isComplete()) {
			  FindPathVisitor lpath(var, source, error, path, stop);
			  l->accept(lpath);
			}
		  }		  
		  break;
		}
	  }
	}
	
  }
  else {
	// does not have 2 children so just go down whichever path
	return true;
  }
  
  return false;
}

  
/*!
 * @return true to continue visiting children, false to stop.
 */
bool FindPathVisitor::visitCombine( wali::witness::WitnessCombine * w ) {

  witness_ls_t& ls = w->children();
  witness_ls_t::iterator it = ls.begin();
  witness_ls_t::iterator itEND = ls.end();

  if (! path.isComplete()) {
	for( ; it != itEND ; it++ ) {
	  witness_t combChild = *it;
	  ErrorPropagation *weightChild = witnessToWeight(combChild.get_ptr());
	  
	  // is this the child we are looking for?
	  if (weightChild->hasError(target, source)) {
		combChild->accept(*this);
		break;
	  }
	}
  }

  // we are handling recursing ourself
  return false;
}


/*!
 * @return true to continue visiting children, false to stop.
 */
bool FindPathVisitor::visitRule( wali::witness::WitnessRule * w ) {

  if (! path.isComplete()) {
	ErrorPropagation* weight = witnessToWeight(w);
	
	// Adding to path...
	bool slice = true;
	MsgRef msg = weight->getMessage(target, source, error, slice, stop);
	
	if (stop) {		
	  path.setComplete(true);
	}

	if (msg->type != Message::Types::EMPTY) {
	  path.add(msg, slice);
	}
  }

  return false;
}


/*!
 * @return true to continue visiting children, false to stop.
 */
bool FindPathVisitor::visitTrans( wali::witness::WitnessTrans * w ATTR_UNUSED) {
  return true;
}


bool FindPathVisitor::visitMerge( wali::witness::WitnessMerge * w ) {

  if( w->hasCaller() && w->hasCallee() ) {
	witness_t l = w->caller(); // witness for caller, before call
	witness_t r = w->callee(); // witness for callee
	witness_t witRule = w->rule(); // witness for call itself
	
	ErrorPropagation* weight = witnessToWeight(w);
	ErrorPropagation* wr = witnessToWeight(r.get_ptr());
	ErrorPropagation* wl = witnessToWeight(l.get_ptr());
	ErrorPropagation* wrule = witnessToWeight(witRule.get_ptr());


	 // transforming errors from tentative to non-tentative
	if (!hexastore) {

	  const string &targetId = variables->getId(target);
	  string::size_type loc = targetId.find("$return", 0);
	  
	  if (loc != string::npos) {
		
		int retIndex = variables->getIndex(weight->getFunction() + "$return");
		
		// function may be void
		if (retIndex != -1) {
		  
		  // is this a non-tentative error?
		  bdd bddError = fdd_ithvar(1, source);
		  if ((bddError & bddNonTentativeErrors) != bddfalse) {
			bdd retToNonTentativeError = (fdd_ithvar(0, retIndex) & fdd_ithvar(1, source));
			bdd retToTentativeError = (fdd_ithvar(0, retIndex) & fdd_ithvar(1, mapRealToTentative[source]));
			
			weight = witnessToWeight(r.get_ptr());	
			if (((weight->BDD & retToNonTentativeError) == bddfalse) &&
				((weight->BDD & retToTentativeError) != bddfalse) ) {
			  // need to switch from non-tentative to tentative
			  source = mapRealToTentative[source];
			  error = source;
			}
		  }	  
		}
	  }

	} // !hexastore


	// continuing constructing sample path

	if (! variables->isGlobal(target)) {

	  // case: local variables

	  vector<int> constants = wr->getConstants(target);
	  vector<int>::iterator it = constants.begin();
	  
	  for(; it != constants.end(); it++) {
		int constant = *it;
		

		int u = variables->getIndex("UNINITIALIZED");
		FindPathVisitor rpath(target, constant, error, path, stop);

		if (constant != u) {
		  r->accept(rpath);
		}
		
		witRule->accept(*this);
		l->accept(*this);
		
		break;
	  }
	}

	else {

	  if (wr->hasError(target, source)) {
		
		// case: global variable
		
		if (target != source) {
		  r->accept(*this);
		}
		
		if (!path.isComplete()) {
		  witRule->accept(*this);
		  
		  if (!path.isComplete()) {
			l->accept(*this);
		  }
		}
	  }
	  
	  else {
		
		// case: formal variable (1)
		
		vector<int> sources = wr->getVariables(target);
		vector<int>::iterator it = sources.begin();
		
		for(; it != sources.end(); it++) {
		  
		  int source1 = *it;
		  if (wrule->hasError(source1, source)) {
			
			FindPathVisitor rpath(target, source1, error, path, stop);
			r->accept(rpath);
			
			FindPathVisitor wpath(source1, source, error, path, stop);
			witRule->accept(wpath);
			
			// done
			break;
		  }
		  else {
			
			// case: formal variable (2)
			
			vector<int> sources2 = wrule->getVariables(source1);
			vector<int>::iterator it = sources2.begin();
			
			for(; it != sources2.end(); it++) {
			  
			  int source2 = *it;
			  
			  if (wl->hasError(source2, source)) {
				
				FindPathVisitor rpath(target, source1, error, path, stop);
				r->accept(rpath);
				
				FindPathVisitor wpath(source1, source2, error, path, stop);
				witRule->accept(wpath);
				
				FindPathVisitor lpath(source2, source, error, path, stop);
				l->accept(lpath);
				
				break;
				
			  }
			}
		  }
		}		
	  }
	}
  }
  else {
	// does not have 2 children so just go down whichever path
	return true; // it was true
  }
  
  return false;
}


ErrorPropagation* FindPathVisitor::witnessToWeight( wali::witness::Witness* w ) {

  ErrorPropagation* weight = static_cast<ErrorPropagation*>(w->weight().get_ptr());
  if( 0 == weight ) {
	std::cerr << "[? ERROR ?] FindPathVisitor::visitExtend\n";
	abort();
  }
  return weight;
}

/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
 */
