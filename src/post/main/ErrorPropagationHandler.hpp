#ifndef ERROR_PROPAGATION_HANDLER_GUARD
#define ERROR_PROPAGATION_HANDLER_GUARD 1

#include "wali/IUserHandler.hpp"
#include "StrX.hpp"
#include "Variables.hpp"
#include "fdd.h"
#include "ErrorPropagation.hpp"
#include "wali/Key.hpp"
#include "ProgramPoint.hpp"
#include "wali/ref_ptr.hpp"
#include "wali/witness/WitnessWrapper.hpp"
#include "TranscodedString.hpp"

#if __cplusplus > 199711L
#  include <memory>
#else
#  include <tr1/memory>
namespace std {
  using std::tr1::shared_ptr;
}
#endif

using wali::witness::WitnessWrapper;
using wali::ref_ptr;
using wali::SemElem;
using wali::MergeFn;
using wali::sem_elem_t;
using wali::merge_fn_t;
using wali::Key;


class ErrorPropagationHandler : public wali::IUserHandler {

public:
  ErrorPropagationHandler();

  //~ErrorPropagationHandler() {}

  virtual void startElement(  
			    const   XMLCh* const    uri,
			    const   XMLCh* const    localname,
			    const   XMLCh* const    qname,
			    const   Attributes&     attributes);

  virtual void endElement( 
			  const XMLCh* const uri
			  , const XMLCh* const localname
			  , const XMLCh* const qname);

  virtual void characters(
			  const XMLCh* const chars
			  , const unsigned int length);
  
  bool handlesElement( std::string tag );
  ref_ptr<wali::wpds::Wrapper> getWrapper();

  sem_elem_t getWeight();
  sem_elem_t getWeight(std::string msgcode, Key fromStkKey);
  void createPoint(std::string msgcode, Key fromStkKey);
  bool hasMergeFn();
  merge_fn_t getMergeFn(sem_elem_t se, Key fromStkKey);
  void setCaller(std::string c);
  void setCallee(std::string c);

protected:
  std::string file;
  int line;
  int locals;
  bool useWeight;
  std::string weightString1;
  std::string weightString2;
  std::string trustedString;
  std::string value;
  std::string name;

  // For predicate tag
  std::string predOp1;
  std::string predOp2;

  //! for getting attrs out of weight
  const TranscodedString setToID;
  const TranscodedString setFromID;
  const TranscodedString trustedID;
  const TranscodedString lineID;
  const TranscodedString fileID;
  const TranscodedString localsID;
  const TranscodedString globalsID;
  const TranscodedString pointersID;
  const TranscodedString basisID;
  const TranscodedString varIdID;
  const TranscodedString varNameID;
  const TranscodedString valueID;
  const TranscodedString predOp1ID;
  const TranscodedString predOp2ID;

  //! more place holders
  StrX setTo;
  StrX setFrom;
  StrX trusted;

private:
  void handleSet(
		 const XMLCh* const uri
		 , const XMLCh* const localname
		 , const XMLCh* const qname
		 , const Attributes& attributes);

  void handleSource(
		    const XMLCh* const uri
		    , const XMLCh* const localname
		    , const XMLCh* const qname
		    , const Attributes& attributes);

  void handlePred(
		    const XMLCh* const uri
		    , const XMLCh* const localname
		    , const XMLCh* const qname
		    , const Attributes& attributes);

  void handleVar(
		 const XMLCh* const uri
		 , const XMLCh* const localname
		 , const XMLCh* const qname
		 , const Attributes& attributes);

  void handleVariables(
		       const XMLCh* const uri
		       , const XMLCh* const localname
		       , const XMLCh* const qname
		       , const Attributes& attributes);

  void handleWeight(
		    const XMLCh* const uri
		    , const XMLCh* const localname
		    , const XMLCh* const qname
		    , const Attributes& attributes);

  void handleReturn(
		 const XMLCh* const uri
		 , const XMLCh* const localname
		 , const XMLCh* const qname
		 , const Attributes& attributes);

  void handleDereference(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

  void handleIsErr(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

  void handleHandled(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

 void handleOperand(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

 void handleInput(
	const XMLCh* const uri ATTR_UNUSED
	, const XMLCh* const localname ATTR_UNUSED
	, const XMLCh* const qname ATTR_UNUSED
	, const Attributes& attributes);

 void handleOutput(
		 const XMLCh* const uri
		 , const XMLCh* const localname
		 , const XMLCh* const qname
		 , const Attributes& attributes);


  void addWeight(string w1, string w2, bool t);

  ErrorPropagation factory;
  sem_elem_t parsedWeight;
  std::string caller;
  std::string callee;
  static bool inSet;
  static bool inReturn;
  static bool inDereference;
  static bool inIsErr;
  static bool inHandled;
  static bool inOperand;
  static bool inInput;
  static bool inOutput;
  static bool inGlobals;
  static bool inLocals;
  static bool inPointers;
  static bool inPred; 
};


extern bdd identity;
extern bdd localsToIdentity;
extern bdd localsToUninitialized;
extern bdd globalsToUniverse;
extern bdd globalsToIdentity;
extern bdd globalsToOK;
extern bdd globalsToUninitialized;
extern bdd constantsToIdentity;

extern map<int, int> mapTentativeToReal;
extern map<int, int> mapRealToTentative;

typedef std::vector<std::shared_ptr<ProgramPoint> > ProgramPoints;
extern ProgramPoints programPoints;
extern set<wali::Key> setProgramPoints;


#endif // ERROR_PROPAGATION_HANDLER_GUARD
